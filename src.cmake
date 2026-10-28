set(PROJECT_INCLUDES
        src
        src/adc
        src/chart
        src/dp_dump
        src/dpt
        src/misc
        src/misc/dbg
        src/mw
        src/other
        src/network
        src/table
        src/thr
)

set(PROJECT_SOURCES
        src/main.cpp
        src/ok.h
        src/ver.h

        src/chart/chart.h
        src/chart/chart.cpp
        src/chart/chart_events.cpp

        src/adc/adc.h
        src/adc/adc.cpp
        src/adc/adc_gui.cpp

        src/dp_dump/dp_dump.h
        src/dp_dump/dp_dump_t.h
        src/dp_dump/dp_dump.cpp
        src/dp_dump/dp_dump_view.h
        src/dp_dump/dp_dump_view.cpp
        src/dp_dump/dp_dump_view_gui.cpp
        src/dp_dump/dp_dump_view_fonts.cpp
        src/dpt/dpt.h
        src/dpt/dpt.cpp
        src/dpt/dpt_gui.cpp
        src/dpt/dpt_fonts.cpp

        src/misc/cfg.h
        src/misc/cfg.cpp
        src/misc/dbg/dbg.h
        src/misc/dbg/dbg_impl.h
        src/misc/dbg/dbg_impl.cpp
        src/misc/lineedit.h
        src/misc/lineedit.cpp
        src/misc/utils.h
        src/misc/utils.cpp

        src/mw/create_menu.cpp
        src/mw/mw.h
        src/mw/mw.cpp
        src/mw/mw_gui.cpp
        src/mw/slot_fonts.cpp
        src/mw/settings.cpp
        src/mw/tr_TR.h
        src/mw/tr_TR.cpp
        src/mw/tr_TR.h
        src/mw/wmsg.h
        src/mw/wmsg.cpp

        src/other/rawfile.h
        src/other/rawfile.cpp

        src/network/packet_aps_t.h
        src/network/packet_aps.cpp
        src/network/packet_atlas_t.h
        src/network/packet_atlas.cpp
        src/network/network.h
        src/network/network.cpp
        src/network/network_run.cpp
        src/network/network_socket.cpp
        src/network/channels_points.h
        src/network/wsa.h
        src/network/wsa.cpp

        src/table/table.h
        src/table/table.cpp

        src/thr/thr.h
        src/thr/thr_t.h
        src/thr/thr.cpp
)
