#-------------------------------------------------------------------------------------
# This is part of CMake configuration file for building makefiles and installfiles
# for the Lomse library
#-------------------------------------------------------------------------------------
# This moule is for defining the source files to compile for building the library
#
#-------------------------------------------------------------------------------------


# source files to compile

set(AGG_FILES
    ${LOMSE_SRC_DIR}/agg/src/agg_arc.cpp
    ${LOMSE_SRC_DIR}/agg/src/agg_bezier_arc.cpp
    ${LOMSE_SRC_DIR}/agg/src/agg_curves.cpp
    ${LOMSE_SRC_DIR}/agg/src/agg_gsv_text.cpp
    ${LOMSE_SRC_DIR}/agg/src/agg_line_aa_basics.cpp
    ${LOMSE_SRC_DIR}/agg/src/agg_rounded_rect.cpp
    ${LOMSE_SRC_DIR}/agg/src/agg_trans_affine.cpp
    ${LOMSE_SRC_DIR}/agg/src/agg_vcgen_contour.cpp
    ${LOMSE_SRC_DIR}/agg/src/agg_vcgen_markers_term.cpp
    ${LOMSE_SRC_DIR}/agg/src/agg_vcgen_stroke.cpp
)

set(DOCUMENT_FILES
    ${LOMSE_SRC_DIR}/document/lomse_command.cpp
    ${LOMSE_SRC_DIR}/document/lomse_document.cpp
    ${LOMSE_SRC_DIR}/document/lomse_document_cursor.cpp
    ${LOMSE_SRC_DIR}/document/lomse_document_iterator.cpp
    ${LOMSE_SRC_DIR}/document/lomse_id_assigner.cpp
)

set(EXPORTERS_FILES
    ${LOMSE_SRC_DIR}/exporters/lomse_ldp_exporter.cpp
    ${LOMSE_SRC_DIR}/exporters/lomse_lmd_exporter.cpp
    ${LOMSE_SRC_DIR}/exporters/lomse_mnx_exporter.cpp
)

set(FILE_SYSTEM_FILES
    ${LOMSE_SRC_DIR}/file_system/lomse_file_system.cpp
    ${LOMSE_SRC_DIR}/file_system/lomse_image_reader.cpp
    ${LOMSE_SRC_DIR}/file_system/lomse_zip_stream.cpp
)

set(GRAPHIC_MODEL_FILES
    ${LOMSE_SRC_DIR}/graphic_model/lomse_box_score_page.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_box_slice.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_box_slice_instr.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_box_system.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_caret.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_caret_positioner.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_engravers_map.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_fragment_mark.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_glyphs.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_gm_basic.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_graphical_model.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_handler.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_gm_measures_table.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_overlays_generator.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_selections.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_shape_barline.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_shape_base.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_shape_beam.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_shape_brace_bracket.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_shape_line.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_shape_note.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_shape_octave_shift.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_shape_staff.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_shape_text.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_shape_tie.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_shape_tuplet.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_shape_volta_bracket.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_shape_wedge.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_shapes.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_sizers.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_tempo_line.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_time_grid.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_timegrid_table.cpp
    ${LOMSE_SRC_DIR}/graphic_model/lomse_visual_effect.cpp
    
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_accidentals_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_articulation_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_beam_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_barline_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_chord_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_clef_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_coda_segno_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_dynamics_mark_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_engrouters.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_fermata_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_instrument_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_key_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_line_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_lyric_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_metronome_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_note_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_octave_shift_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_ornament_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_rest_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_slur_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_technical_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_text_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_tie_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_time_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_tuplet_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_volta_engraver.cpp
    ${LOMSE_SRC_DIR}/graphic_model/engravers/lomse_wedge_engraver.cpp

    ${LOMSE_SRC_DIR}/graphic_model/layouters/lomse_blocks_container_layouter.cpp
    ${LOMSE_SRC_DIR}/graphic_model/layouters/lomse_document_layouter.cpp
    ${LOMSE_SRC_DIR}/graphic_model/layouters/lomse_inlines_container_layouter.cpp
    ${LOMSE_SRC_DIR}/graphic_model/layouters/lomse_layouter.cpp
    ${LOMSE_SRC_DIR}/graphic_model/layouters/lomse_right_aligner.cpp
    ${LOMSE_SRC_DIR}/graphic_model/layouters/lomse_score_layouter.cpp
    ${LOMSE_SRC_DIR}/graphic_model/layouters/lomse_score_meter.cpp
    ${LOMSE_SRC_DIR}/graphic_model/layouters/lomse_spacing_algorithm.cpp
    ${LOMSE_SRC_DIR}/graphic_model/layouters/lomse_spacing_algorithm_gourlay.cpp
    ${LOMSE_SRC_DIR}/graphic_model/layouters/lomse_staffobjs_cursor.cpp
    ${LOMSE_SRC_DIR}/graphic_model/layouters/lomse_system_layouter.cpp
    ${LOMSE_SRC_DIR}/graphic_model/layouters/lomse_table_layouter.cpp
    ${LOMSE_SRC_DIR}/graphic_model/layouters/lomse_text_splitter.cpp
    ${LOMSE_SRC_DIR}/graphic_model/layouters/lomse_vertical_profile.cpp
)

set(GUI_CONTROLS_FILES
    ${LOMSE_SRC_DIR}/gui_controls/lomse_button_ctrl.cpp
    ${LOMSE_SRC_DIR}/gui_controls/lomse_checkbox_ctrl.cpp
    ${LOMSE_SRC_DIR}/gui_controls/lomse_hyperlink_ctrl.cpp
    ${LOMSE_SRC_DIR}/gui_controls/lomse_score_player_ctrl.cpp
    ${LOMSE_SRC_DIR}/gui_controls/lomse_static_text_ctrl.cpp
    ${LOMSE_SRC_DIR}/gui_controls/lomse_progress_bar_ctrl.cpp
)

set(INTERNAL_MODEL_FILES
    ${LOMSE_SRC_DIR}/internal_model/lomse_api_internal_model.cpp
    ${LOMSE_SRC_DIR}/internal_model/lomse_im_algorithms.cpp
    ${LOMSE_SRC_DIR}/internal_model/lomse_im_attributes.cpp
    ${LOMSE_SRC_DIR}/internal_model/lomse_im_factory.cpp
    ${LOMSE_SRC_DIR}/internal_model/lomse_im_figured_bass.cpp
    ${LOMSE_SRC_DIR}/internal_model/lomse_im_measures_table.cpp
    ${LOMSE_SRC_DIR}/internal_model/lomse_im_note.cpp
    ${LOMSE_SRC_DIR}/internal_model/lomse_internal_model.cpp
    ${LOMSE_SRC_DIR}/internal_model/lomse_model_builder.cpp
    ${LOMSE_SRC_DIR}/internal_model/lomse_score_algorithms.cpp
    ${LOMSE_SRC_DIR}/internal_model/lomse_score_utilities.cpp
    ${LOMSE_SRC_DIR}/internal_model/lomse_staffobjs_table.cpp
)

set(MODULE_FILES
    ${LOMSE_SRC_DIR}/module/lomse_doorway.cpp
    ${LOMSE_SRC_DIR}/module/lomse_events.cpp
    ${LOMSE_SRC_DIR}/module/lomse_events_dispatcher.cpp
    ${LOMSE_SRC_DIR}/module/lomse_image.cpp
    ${LOMSE_SRC_DIR}/module/lomse_injectors.cpp
    ${LOMSE_SRC_DIR}/module/lomse_interval.cpp
    ${LOMSE_SRC_DIR}/module/lomse_logger.cpp
    ${LOMSE_SRC_DIR}/module/lomse_pitch.cpp
    ${LOMSE_SRC_DIR}/module/lomse_time.cpp
)

set(MVC_FILES
    ${LOMSE_SRC_DIR}/mvc/lomse_graphic_view.cpp
    ${LOMSE_SRC_DIR}/mvc/lomse_half_page_view.cpp
    ${LOMSE_SRC_DIR}/mvc/lomse_interactor.cpp
    ${LOMSE_SRC_DIR}/mvc/lomse_presenter.cpp 
    ${LOMSE_SRC_DIR}/mvc/lomse_tasks.cpp
    ${LOMSE_SRC_DIR}/mvc/lomse_view.cpp
)

set(PARSER_FILES
    ${LOMSE_SRC_DIR}/parser/lomse_analyser.cpp
    ${LOMSE_SRC_DIR}/parser/lomse_autobeamer.cpp
    ${LOMSE_SRC_DIR}/parser/lomse_autoclef.cpp
    ${LOMSE_SRC_DIR}/parser/lomse_compiler.cpp
    ${LOMSE_SRC_DIR}/parser/lomse_ldp_elements.cpp
    ${LOMSE_SRC_DIR}/parser/lomse_ldp_factory.cpp
    ${LOMSE_SRC_DIR}/parser/lomse_linker.cpp
    ${LOMSE_SRC_DIR}/parser/lomse_reader.cpp
    ${LOMSE_SRC_DIR}/parser/lomse_tokenizer.cpp
    ${LOMSE_SRC_DIR}/parser/lomse_xml_parser.cpp

    ${LOMSE_SRC_DIR}/parser/ldp/lomse_ldp_analyser.cpp
    ${LOMSE_SRC_DIR}/parser/ldp/lomse_ldp_compiler.cpp
    ${LOMSE_SRC_DIR}/parser/ldp/lomse_ldp_parser.cpp

    ${LOMSE_SRC_DIR}/parser/lmd/lomse_lmd_analyser.cpp
    ${LOMSE_SRC_DIR}/parser/lmd/lomse_lmd_compiler.cpp

    ${LOMSE_SRC_DIR}/parser/mxl/lomse_compressed_mxl_compiler.cpp
    ${LOMSE_SRC_DIR}/parser/mxl/lomse_mxl_analyser.cpp
    ${LOMSE_SRC_DIR}/parser/mxl/lomse_mxl_compiler.cpp

    ${LOMSE_SRC_DIR}/parser/mnx/lomse_mnx_analyser.cpp
    ${LOMSE_SRC_DIR}/parser/mnx/lomse_mnx_compiler.cpp
)

set(RENDER_FILES
    ${LOMSE_SRC_DIR}/render/lomse_bitmap_drawer.cpp
    ${LOMSE_SRC_DIR}/render/lomse_calligrapher.cpp
    ${LOMSE_SRC_DIR}/render/lomse_font_freetype.cpp
    ${LOMSE_SRC_DIR}/render/lomse_font_storage.cpp
    ${LOMSE_SRC_DIR}/render/lomse_renderer.cpp
)

set(SCORE_FILES
    ${LOMSE_SRC_DIR}/score/lomse_score_iterator.cpp
)

set(SOUND_FILES
    ${LOMSE_SRC_DIR}/sound/lomse_midi_table.cpp
    ${LOMSE_SRC_DIR}/sound/lomse_score_player.cpp
)

set(LOMSE_PACKAGES_FILES
    ${LOMSE_PKG_DIR}/pugixml/pugixml.cpp
)

if( LOMSE_ENABLE_COMPRESSION )
    set(LOMSE_PACKAGES_FILES
        ${LOMSE_PACKAGES_FILES}
        ${LOMSE_PKG_DIR}/minizip/unzip.c
        ${LOMSE_PKG_DIR}/minizip/ioapi.c
    )
endif()

# platform dependent implementation files
if(UNIX)
    set(PLATFORM_FILES
        ${LOMSE_SRC_DIR}/platform/lomse_linux.cpp
    )
elseif(WIN32)
    set(PLATFORM_FILES
        ${LOMSE_SRC_DIR}/platform/lomse_windows.cpp
    )
else()
    set(PLATFORM_FILES
        ${LOMSE_SRC_DIR}/platform/lomse_other.cpp
    )
endif()


set(ALL_LOMSE_SOURCES 
    ${AGG_FILES} ${DOCUMENT_FILES} ${EXPORTERS_FILES} ${FILE_SYSTEM_FILES}
    ${GRAPHIC_MODEL_FILES} ${GUI_CONTROLS_FILES} ${INTERNAL_MODEL_FILES} 
    ${MODULE_FILES} ${MVC_FILES}
    ${PARSER_FILES} ${PLATFORM_FILES} ${RENDER_FILES} ${SCORE_FILES} 
    ${SOUND_FILES} ${LOMSE_PACKAGES_FILES}
)


