/* ide-xml-position.c
 *
 * Copyright (C) 2017 Sebastien Lafargue <slafargue@gnome.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ide-xml-position.h"

G_DEFINE_BOXED_TYPE (IdeXmlPosition, ide_xml_position, ide_xml_position_ref, ide_xml_position_unref)

IdeXmlPosition *
ide_xml_position_new (IdeXmlSymbolNode   *node,
                      IdeXmlPositionKind  kind)
{
  IdeXmlPosition *self;

  g_return_val_if_fail (IDE_IS_XML_SYMBOL_NODE (node), NULL);

  self = g_slice_new0 (IdeXmlPosition);
  self->ref_count = 1;

  self->node = (IDE_IS_XML_SYMBOL_NODE (node)) ? g_object_ref (node) : NULL;
  self->kind = kind;
  self->child_pos = -1;

  return self;
}

IdeXmlPosition *
ide_xml_position_copy (IdeXmlPosition *self)
{
  IdeXmlPosition *copy;

  g_return_val_if_fail (self, NULL);
  g_return_val_if_fail (self->ref_count, NULL);

  copy = ide_xml_position_new (self->node,
                               self->kind);

  if (self->analysis != NULL)
    copy->analysis = ide_xml_analysis_ref (self->analysis);

  if (self->next_sibling_node != NULL)
    copy->next_sibling_node = g_object_ref (self->next_sibling_node);

  if (self->previous_sibling_node != NULL)
    copy->previous_sibling_node = g_object_ref (self->previous_sibling_node);

  copy->child_pos = self->child_pos;

  return copy;
}

static void
ide_xml_position_free (IdeXmlPosition *self)
{
  g_assert (self);
  g_assert_cmpint (self->ref_count, ==, 0);

  if (self->analysis != NULL)
    ide_xml_analysis_unref (self->analysis);

  g_clear_object (&self->node);
  g_clear_object (&self->previous_sibling_node);
  g_clear_object (&self->next_sibling_node);

  g_slice_free (IdeXmlPosition, self);
}

IdeXmlPosition *
ide_xml_position_ref (IdeXmlPosition *self)
{
  g_return_val_if_fail (self, NULL);
  g_return_val_if_fail (self->ref_count, NULL);

  g_atomic_int_inc (&self->ref_count);

  return self;
}

void
ide_xml_position_unref (IdeXmlPosition *self)
{
  g_return_if_fail (self);
  g_return_if_fail (self->ref_count);

  if (g_atomic_int_dec_and_test (&self->ref_count))
    ide_xml_position_free (self);
}

IdeXmlAnalysis *
ide_xml_position_get_analysis (IdeXmlPosition *self)
{
  g_return_val_if_fail (self, NULL);

  return self->analysis;
}

void
ide_xml_position_set_analysis (IdeXmlPosition *self,
                               IdeXmlAnalysis *analysis)
{
  g_return_if_fail (self);

  self->analysis = ide_xml_analysis_ref (analysis);
}

IdeXmlSymbolNode *
ide_xml_position_get_next_sibling (IdeXmlPosition *self)
{
  g_return_val_if_fail (self, NULL);

  return self->next_sibling_node;
}

IdeXmlSymbolNode *
ide_xml_position_get_previous_sibling (IdeXmlPosition *self)
{
  g_return_val_if_fail (self, NULL);

  return self->previous_sibling_node;
}

void
ide_xml_position_set_siblings    (IdeXmlPosition   *self,
                                  IdeXmlSymbolNode *previous_sibling_node,
                                  IdeXmlSymbolNode *next_sibling_node)
{
  if (previous_sibling_node != NULL)
    g_object_ref (previous_sibling_node);

  if (next_sibling_node != NULL)
    g_object_ref (next_sibling_node);

  self->previous_sibling_node = previous_sibling_node;
  self->next_sibling_node = next_sibling_node;
}

const gchar *
ide_xml_position_kind_get_str (IdeXmlPositionKind kind)
{
  const gchar *kind_str;

  switch (kind)
    {
    case IDE_XML_POSITION_KIND_UNKNOW:
      kind_str = "unknow";
      break;

    case IDE_XML_POSITION_KIND_IN_START_TAG:
      kind_str = "in start";
      break;

    case IDE_XML_POSITION_KIND_IN_END_TAG:
      kind_str = "in end";
      break;

    case IDE_XML_POSITION_KIND_IN_CONTENT:
      kind_str = "in content";
      break;

    default:
      g_assert_not_reached ();
    }

  return kind_str;
}

void
ide_xml_position_print (IdeXmlPosition *self)
{
  const gchar *p_sibling_str;
  const gchar *n_sibling_str;
  const gchar *kind_str;
  IdeXmlSymbolNode *parent_node;
  gint n_children;

  p_sibling_str = (self->previous_sibling_node == NULL) ?
    "none" :
    ide_xml_symbol_node_get_element_name (self->previous_sibling_node);

  n_sibling_str = (self->next_sibling_node == NULL) ?
    "none" :
    ide_xml_symbol_node_get_element_name (self->next_sibling_node);

  kind_str = ide_xml_position_kind_get_str (self->kind);

  parent_node = ide_xml_symbol_node_get_parent (self->node);
  printf ("POSITION: parent: %s node: %s kind:%s",
          (parent_node != NULL) ? ide_xml_symbol_node_get_element_name (parent_node) : "none",
          (self->node != NULL) ? ide_xml_symbol_node_get_element_name (self->node) : "none",
          kind_str);

  if (self->child_pos != -1)
    {
      printf (" (between %s and %s)", p_sibling_str, n_sibling_str);
      n_children = ide_xml_symbol_node_get_n_direct_children (self->node);
      if (self->child_pos == 0)
        {
          if (n_children == 1)
            printf (" pos: |0\n");
          else
            printf (" pos: |0..%d\n", n_children - 1);
        }
      else if (self->child_pos == (n_children + 1))
        {
          if (n_children == 1)
            printf (" pos: 0|\n");
          else
            printf (" pos: 0..%d|\n", n_children - 1);
        }
      else
        printf (" pos: %d|%d\n", self->child_pos - 1, self->child_pos);
    }
  else
    printf ("\n");

  if (self->node != NULL)
    {
      const gchar **attributes_names;

      if (NULL != (attributes_names = ide_xml_symbol_node_get_attributes_names (self->node)))
        {
          while (attributes_names [0] != NULL)
            {
              printf ("attr:%s\n", *attributes_names);
              ++attributes_names;
            }
        }
    }
}

IdeXmlSymbolNode *
ide_xml_position_get_node (IdeXmlPosition *self)
{
  g_return_val_if_fail (self, NULL);

  return self->node;
}

IdeXmlPositionKind
ide_xml_position_get_kind (IdeXmlPosition *self)
{
  g_return_val_if_fail (self, IDE_XML_POSITION_KIND_UNKNOW);

  return self->kind;
}

gint
ide_xml_position_get_child_pos (IdeXmlPosition *self)
{
  g_return_val_if_fail (self, -1);

  return self->child_pos;
}

void ide_xml_position_set_child_pos (IdeXmlPosition *self,
                                     gint            child_pos)
{
  g_return_if_fail (self);

  self->child_pos = child_pos;
}
