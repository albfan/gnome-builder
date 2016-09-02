# Debugger Overview

The goal of this plugin is to reuse as much of the Nemiver codebase as we can
to implement a debugger in Builder. Rather than abstract debugger
implementations in Builder, we will rely on Nemiver's debugger abstraction.
That means to add a new debugger to Builder, you need to add a new debugger
module to Nemiver. This is completely a reasonable request as it allows for
Nemiver to be more generally useful as a standalone debugger for those that
prefer standalone tools.

## Debugger Service

This `IService` implementation bridges the Nemiver IDebugger implementation
with Builder. It provides the API calls necessary to make progress on the
debugger as well register breakpoints, watchpoints, countpoints, etc. These API
calls are used by the `EditorViewAddin` (and associated GutterRenderer) to
toggle breakpoints on a given line. Additionally, the service provides
information on the current position of the debug cycle, so that we can show an
arrow for the current position in the gutter.

To simplify interaction with UI elements, the Debugger service also exposes
a `GActionGroup` with actions that can be activated from Gtk widgetry. The
enabled-state of the actions are automatically updated based on the bridged
`IDebugger`s ability to perform the action.

## Debugger Perspective

The debugger perspective imports the widgetry from Nemiver into a layout
suitable for debugging in Builder. It includes a location for read-only
editors (of the file for the current debugged line) as well as the panels
such as memory, threads, breakpoints, etc.

## Debugger EditorViewAddin

This addin hooks into the `IdeEditorView` to add the debugger gutter to sources
that can have breakpoints added to them. Such an example would be C and C++
files. As additional debugger backends are added, this will need to be
extended to enable other language ids.

## Debugger GutterRenderer

This is a `GtkSourceGutterRenderer` that allows toggling breakpoints (as well
as countpoints) for a given line in a source file.

## Debugger WorkbenchAddin

The `IdeWorkbenchAddin` is used to add the perspective to the workbench. Also,
it adds the Debugger service to the toplevel as an action group so that buttons
can be attached to the actions. The debugger controls are added to the UI here
as well, and this class manages their visibility (based on the debugger service
actively debugging).

This also registers the `IdeRunHandler` for the debugger. When activated, it
requests the debugger service to start the debugger using the `IdeRunner` for
the run operation.

## Debugger Breakpoints

Because GtkSourceGutterRenderers do not have their contents cached, we need a
*very* fast way to determine if a breakpoint is on a given source line. The
`GbpDebuggerBreakpoints` provides that feature by caching what lines have
breakpoints. This allows the gutter renderer to quickly check for every line
rendered as well as invalidating the window when breakpoints change.
