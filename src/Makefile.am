bin_PROGRAMS = loopback rmap rd_rmap stipa la_routing load
loopback_SOURCES = test_loopback.c utility.c
loopback_LDADD = -lstar_conf_api_brick_mk3 -lstar_conf_api_mk2 -lstar_conf_api_router -lstar-api

rmap_SOURCES = test_rmap.c utility.c
rmap_LDADD = -lstar_conf_api_brick_mk3 -lstar_conf_api_mk2 -lstar_conf_api_router -lstar-api -lrmap_packet_library

rd_rmap_SOURCES = test_read_rmap.c utility.c
rd_rmap_LDADD =  -lstar_conf_api_brick_mk3 -lstar_conf_api_mk2 -lstar_conf_api_router -lstar-api -lrmap_packet_library

stipa_SOURCES = stipa.c utility.c
stipa_LDADD =  -lstar_conf_api_brick_mk3 -lstar_conf_api_mk2 -lstar_conf_api_router -lstar-api -lrmap_packet_library

la_routing_SOURCES = test_la_routing.c utility.c
la_routing_LDADD = -lstar_conf_api_brick_mk3 -lstar_conf_api_mk2 -lstar_conf_api_router -lstar-api -lrmap_packet_library

load_SOURCES = load_reg.c utility.c
load_LDADD =  -lstar_conf_api_brick_mk3 -lstar_conf_api_mk2 -lstar_conf_api_router -lstar-api -lrmap_packet_library
