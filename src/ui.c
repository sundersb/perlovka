/*
    Perlovka - grain reduction filter
    Copyright (C) 2025 Alexander Belkov

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "ui.h"

gboolean show_perlovka_dialog(PerlovkaPluginSettings *settings)
{
    GtkWidget *dlg;
    GtkWidget *main_vbox;
    GtkWidget *frame;
    GtkWidget *table;
    GtkWidget *spin;
    GtkWidget *combo;
    GtkWidget *check;
    GtkAdjustment *adj;
    gboolean result;

    gimp_ui_init(PLUGIN_NAME, TRUE);

    dlg = gimp_dialog_new(_("Perlovka Degranulation Filter"), PLUGIN_NAME,
                          NULL, 0,
                          gimp_standard_help_func, PLUG_IN_PROC,
                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                          GTK_STOCK_OK, GTK_RESPONSE_OK,
                          NULL);

    main_vbox = gtk_vbox_new(FALSE, 12);
    gtk_container_set_border_width(GTK_CONTAINER(main_vbox), 12);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dlg)->vbox), main_vbox);

    frame = gimp_frame_new(_("Perlovka Settings"));
    gtk_box_pack_start(GTK_BOX(main_vbox), frame, FALSE, FALSE, 0);
    gtk_widget_show(frame);

    table = gtk_table_new(6, 2, FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(table), 4);
    gtk_table_set_col_spacings(GTK_TABLE(table), 4);
    gtk_table_set_row_spacings(GTK_TABLE(table), 2);
    gtk_container_add(GTK_CONTAINER(frame), table);
    gtk_widget_show(table);

    adj = (GtkAdjustment *)gtk_adjustment_new(settings->radius, 1, 50, 1, 10, 0);
    spin = gimp_spin_button_new(adj, 1.0, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spin), TRUE);
    gimp_table_attach_aligned(GTK_TABLE(table), 0, 0, _("_Radius:"), 0.0, 0.5, spin, 1, TRUE);
    g_signal_connect(adj, "value-changed",
                     G_CALLBACK(gimp_int_adjustment_update),
                     &settings->radius);

    adj = (GtkAdjustment *)gtk_adjustment_new(settings->iterations_limit, 1, 100, 1, 10, 0);
    spin = gimp_spin_button_new(adj, 1.0, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spin), TRUE);
    gimp_table_attach_aligned(GTK_TABLE(table), 0, 1, _("_Iterations:"), 0.0, 0.5, spin, 1, TRUE);
    g_signal_connect(adj, "value-changed",
                     G_CALLBACK(gimp_int_adjustment_update),
                     &settings->iterations_limit);

    combo = gimp_int_combo_box_new("Odd", 0, "Even", 1, "Both", 2, NULL);
    gimp_table_attach_aligned(GTK_TABLE(table), 0, 2, _("Grid:"), 0.0, 0.5, combo, 1, FALSE);
    gimp_int_combo_box_connect(GIMP_INT_COMBO_BOX(combo),
                               settings->grid,
                               G_CALLBACK(gimp_int_combo_box_get_active),
                               &settings->grid);

    combo = gimp_int_combo_box_new("Soft", 0, "Strict", 1, NULL);
    gimp_table_attach_aligned(GTK_TABLE(table), 0, 3, _("Matching:"), 0.0, 0.5, combo, 1, FALSE);
    gimp_int_combo_box_connect(GIMP_INT_COMBO_BOX(combo),
                               settings->matching,
                               G_CALLBACK(gimp_int_combo_box_get_active),
                               &settings->matching);

    combo = gimp_int_combo_box_new("Minimal", 0, "Least of Max", 1, "Largest of Min", 2, "Maximal", 3, NULL);
    gimp_table_attach_aligned(GTK_TABLE(table), 0, 4, _("Resolver:"), 0.0, 0.5, combo, 1, FALSE);
    gimp_int_combo_box_connect(GIMP_INT_COMBO_BOX(combo),
                               settings->resolver,
                               G_CALLBACK(gimp_int_combo_box_get_active),
                               &settings->resolver);

    check = gtk_check_button_new_with_label("Field matching");
    gimp_table_attach_aligned(GTK_TABLE(table), 0, 5, NULL, 0.0, 0.5, check, 1, FALSE);
    g_signal_connect(check, "toggled",
                     G_CALLBACK(gimp_toggle_button_update),
                     &settings->field_matching);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), settings->field_matching);

    gtk_widget_show(main_vbox);
    gtk_widget_show(dlg);

    result = (gimp_dialog_run(GIMP_DIALOG(dlg)) == GTK_RESPONSE_OK);

    gtk_widget_destroy(dlg);

    return result;
}

#pragma GCC diagnostic pop
