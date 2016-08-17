# __init__.py
#
# Copyright (C) 2016 Patrick Griffis <tingping@tingping.se>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from os import path
import threading
import shutil
import json
import gi

gi.require_version('Ide', '1.0')

from gi.repository import (
    GLib,
    GObject,
    Gio,
    Ide
)

_ = Ide.gettext


class MesonBuildSystem(Ide.Object, Ide.BuildSystem, Gio.AsyncInitable):
    project_file = GObject.Property(type=Gio.File)

    def do_init_async(self, priority, cancel, callback, data=None):
        task = Gio.Task.new(self, cancel, callback)
        task.set_priority(priority)

        project_file = self.get_context().get_project_file()
        if project_file.get_basename() == 'meson.build':
            task.return_boolean(True)
        else:
            child = project_file.get_child('meson.build')
            exists = child.query_exists(cancel)
            if exists:
                self.props.project_file = child
            task.return_boolean(exists)

    def do_init_finish(self, result):
        return result.propagate_boolean()

    def do_get_priority(self):
        return -200 # Lower priority than Autotools for now

    def do_get_builder(self, config):
        self._config = config
        return MesonBuilder(context=self.get_context(), configuration=config)

    def do_get_build_flags_async(self, ifile, cancellable, callback):
        task = Gio.Task.new(self, cancellable, callback)
        task.build_flags = []

        if not self._config:
            task.return_boolean(False)
            return

        # TODO
        task.return_boolean(True)

    def do_get_build_flags_finish(self, result):
        return result.build_flags

    def do_get_build_targets_async(self, cancellable, callback, data=None):
        task = Gio.Task.new(self, cancellable, callback)
        task.build_targets = None

        # FIXME: API cleanup for this?
        config = Ide.Configuration.new(self.get_context(), 'meson-bootstrap', 'local', 'host')
        builder = self.get_builder(config)

        import subprocess
        try:
            ret = subprocess.check_output(['mesonintrospect', '--targets', builder._get_build_dir().get_path()])
        except (subprocess.CalledProcessError, FileNotFoundError):
            task.return_error(GLib.Error('Failed to run mesonintrospect'))
            return

        #launcher = Ide.SubprocessLauncher.new(Gio.SubprocessFlags.NONE)
        #launcher.push_args(['mesonintrospect', '--targets', builder._get_build_dir().get_path()])
        #proc = launcher.spawn_sync(cancellable)
        #try:
        #   ret, stdout, stderr = proc.communicate_utf8(None, cancellable)
        #    print(ret, stdout, stderr)
        #    proc.wait(cancellable)
        #except GLib.Error as e:
        #    task.build_targets = None
        #    task.return_error(e)
        #    return

        targets = []
        try:
            meson_targets = json.loads(ret.decode('utf-8'))
        except json.JSONDecodeError:
            task.return_error(GLib.Error('Failed to decode meson json'))
            return

        bindir = path.join(config.get_prefix(), 'bin')
        for t in meson_targets:
            name = t['filename']
            if isinstance(name, list):
                name = name[0]

            install_dir = t.get('install_filename', '')
            installed = t['installed']
            if installed and not install_dir:
                print('Meson: Warning: Older versions of Meson did not expose install dir')
                if t['type'] == 'executable':
                    # Hardcode bad guess
                    install_dir = bindir
            elif install_dir:
                install_dir = path.dirname(install_dir)

            ide_target = MesonBuildTarget(install_dir, name=name)
            # Try to be smart and sort these because Builder runs the
            # first one. Ideally it allows the user to select the run
            # targets.
            if t['type'] == 'executable' and installed and \
                install_dir.startswith(bindir) and not t['filename'].endswith('-cli'):
                targets.insert(0, ide_target)
            else:
                targets.append(ide_target)

        task.build_targets = targets
        task.return_boolean(True)

    def do_get_build_targets_finish(self, result):
        if result.build_targets is None:
            raise result.propagate_error()
        return result.build_targets


class MesonBuilder(Ide.Builder):
    configuration = GObject.Property(type=Ide.Configuration)

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

    def _get_build_dir(self) -> Gio.File:
        context = self.get_context()

        # This matches the Autotools layout
        project_id = context.get_project().get_id()
        buildroot = context.get_root_build_dir()
        device = self.props.configuration.get_device()
        device_id = device.get_id()
        system_type = device.get_system_type()

        return Gio.File.new_for_path(path.join(buildroot, project_id, device_id, system_type))

    def _get_source_dir(self) -> Gio.File:
        context = self.get_context()
        return context.get_vcs().get_working_directory()

    def do_build_async(self, flags, cancellable, callback, data=None):
        task = Gio.Task.new(self, cancellable, callback)
        task.build_result = MesonBuildResult(self.configuration,
                                             self._get_build_dir(),
                                             self._get_source_dir(),
                                             cancellable,
                                             flags=flags)

        def wrap_build():
            task.build_result.set_running(True)
            try:
                task.build_result.build()
                task.build_result.set_mode(_('Successful'))
                task.build_result.set_failed(False)
                task.return_boolean(True)
            except GLib.Error as e:
                task.build_result.set_mode(_('Failed'))
                task.build_result.set_failed(True)
                task.return_error(e)
            task.build_result.set_running(False)

        thread = threading.Thread(target=wrap_build)
        thread.start()

        return task.build_result

    def do_build_finish(self, result) -> Ide.BuildResult:
        if result.propagate_boolean():
            return result.build_result

    def do_install_async(self, cancellable, callback, data=None):
        task = Gio.Task.new(self, cancellable, callback)
        task.build_result = MesonBuildResult(self.configuration,
                                             self._get_build_dir(),
                                             self._get_source_dir(),
                                             cancellable)

        def wrap_install():
            task.build_result.set_running(True)
            try:
                task.build_result.install()
                self = task.get_source_object()
                task.build_result.set_mode(_('Successful'))
                task.build_result.set_failed(False)
                task.return_boolean(True)
            except GLib.Error as e:
                task.build_result.set_mode(_("Failed"))
                task.build_result.set_failed(True)
                task.return_error(e)
            task.build_result.set_running(False)

        thread = threading.Thread(target=wrap_install)
        thread.start()

        return task.build_result

    def do_install_finish(self, result) -> Ide.BuildResult:
        if result.propagate_boolean():
            return result.build_result


class MesonBuildResult(Ide.BuildResult):

    def __init__(self, config, blddir, srcdir, cancel, flags=0, **kwargs):
        super().__init__(**kwargs)
        self.config = config
        self.cancel = cancel
        self.flags = flags
        self.runtime = config.get_runtime()
        self.blddir = blddir
        self.srcdir = srcdir

    def _new_launcher(self, cwd=None):
        if self.runtime:
            launcher = self.runtime.create_launcher()
        else:
            launcher = Ide.SubprocessLauncher.new(Gio.SubprocessFlags.NONE)
            launcher.set_run_on_host(True)
            launcher.set_clear_env(False)
        if cwd:
            launcher.set_cwd(cwd.get_path())
        return launcher

    def _run_subprocess(self, launcher):
        proc = launcher.spawn_sync()
        self.log_subprocess(proc)
        proc.wait_check(self.cancel)

    def install(self):
        launcher = self._new_launcher(cwd=self.blddir)
        launcher.push_args(['ninja', 'install'])
        self._run_subprocess(launcher)

    def build(self):
        """
        NOTE: This is ran in a thread and it raising GLib.Error is
              handled a layer up.
        """
        print(Ide.BuilderBuildFlags(self.flags).value_names)
        clean = bool(self.flags & Ide.BuilderBuildFlags.FORCE_CLEAN)
        build = not self.flags & Ide.BuilderBuildFlags.NO_BUILD
        bootstrap = bool(self.flags & Ide.BuilderBuildFlags.FORCE_BOOTSTRAP)

        if bootstrap or self.config.get_dirty():
            try:
                shutil.rmtree(self.blddir.get_path())
            except FileNotFoundError:
                pass
            self.config.set_dirty(False)

        if not self.blddir.query_exists():
            self.blddir.make_directory_with_parents(self.cancel)

        # TODO: Configuration doesn't quite map well to Meson yet
        # so for example if you have -Dfoo in your extra options
        # and then remove it we can reconfigure but foo will not change.
        # Thus for now we have to take the approach of deleting everything.
        if False: #self.config.get_dirty():
            launcher = self._new_launcher()
            opts = self.config.get_config_opts() or ''
            extra_opts = opts.split() # TODO: Probably want to convert -- flags to -D
            extra_opts.append('-Dprefix=' + self.config.get_prefix())
            launcher.push_args(['mesonconf', self.blddir.get_path()] + extra_opts)
            self._run_subprocess(launcher)
            self.config.set_dirty(False)

        if not self.blddir.get_child('build.ninja').query_exists():
            launcher = self._new_launcher(self.srcdir)

            opts = self.config.get_config_opts() or ''
            extra_opts = opts.split()
            extra_opts.append('--prefix=' + self.config.get_prefix())
            launcher.push_args(['meson', self.blddir.get_path()] + extra_opts)

            self.set_mode(_('Configuring…'))
            subproc = launcher.spawn_sync()
            self.log_subprocess(subproc)
            subproc.wait_check()

        launcher = self._new_launcher(self.blddir)
        launcher.push_args(['ninja']) # FIXME: ninja-build on Fedora
        if clean:
            self.set_mode(_('Cleaning…'))
            launcher.push_args(['clean'])
            self._run_subprocess(launcher)
        if build:
            if clean: # Build after cleaning
                launcher.pop_argv()
            self.set_mode(_('Building…'))
            self._run_subprocess(launcher)

class MesonBuildTarget(Ide.Object, Ide.BuildTarget):
    # FIXME: These should be part of the BuildTarget interface
    name = GObject.Property(type=str)
    install_directory = GObject.Property(type=Gio.File)

    def __init__(self, install_dir, **kwargs):
        super().__init__(**kwargs)
        self.props.install_directory = Gio.File.new_for_path(install_dir)

    def do_get_install_directory(self):
        return self.props.install_directory


#class MesonMiner(Ide.ProjectMiner):
#    def __init__(**kwargs):
#        super().__init__(**kwargs)
#
#    def do_mine_async(self, cancel, callback):
#        pass
#
#    def do_mine_finish(self, result):
#        pass
