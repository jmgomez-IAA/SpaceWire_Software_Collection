bin_PROGRAMS = loopback rmap rd_rmap receiv timecode reset
loopback_SOURCES = test_loopback.c utility.c
loopback_LDADD = -lstar_conf_api_brick_mk3 -lstar_conf_api_mk2 -lstar_conf_api_router -lstar-api

rmap_SOURCES = test_rmap.c utility.c
rmap_LDADD = -lstar_conf_api_brick_mk3 -lstar_conf_api_mk2 -lstar_conf_api_router -lstar-api -lrmap_packet_library

rd_rmap_SOURCES = test_read_rmap.c utility.c
rd_rmap_LDADD =  -lstar_conf_api_brick_mk3 -lstar_conf_api_mk2 -lstar_conf_api_router -lstar-api -lrmap_packet_library

reset_SOURCES = test_reset_dev.c utility.c
reset_LDADD = -lstar_conf_api_brick_mk3 -lstar_conf_api_mk2 -lstar_conf_api_router -lstar-api -lrmap_packet_library

receiv_SOURCES = test_receiv.c utility.c
receiv_LDADD  =  -lstar_conf_api_brick_mk3 -lstar_conf_api_mk2 -lstar_conf_api_router -lstar-api -lrmap_packet_library

timecode_SOURCES = test_timecode.c utility.c
timecode_LDADD  = -lpthread -lstar_conf_api_brick_mk3 -lstar_conf_api_mk2 -lstar_conf_api_router -lstar-api -lrmap_packet_library
