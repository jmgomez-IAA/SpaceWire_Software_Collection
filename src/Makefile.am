bin_PROGRAMS = loopback rmap rd_rmap stipa la_routing route_NDPU load apus la2_routing conf_router receiv timecode
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

la2_routing_SOURCES = test_la2_routing.c utility.c
la2_routing_LDADD = -lstar_conf_api_brick_mk3 -lstar_conf_api_mk2 -lstar_conf_api_router -lstar-api -lrmap_packet_library

load_SOURCES = load_reg.c utility.c
load_LDADD =  -lstar_conf_api_brick_mk3 -lstar_conf_api_mk2 -lstar_conf_api_router -lstar-api -lrmap_packet_library

apus_SOURCES = apus.c utility.c
apus_LDADD = -lpthread -lstar_conf_api_brick_mk3 -lstar_conf_api_mk2 -lstar_conf_api_router -lstar-api -lrmap_packet_library

route_NDPU_SOURCES = test_routing_NDPU.c utility.c
route_NDPU_LDADD  =  -lstar_conf_api_brick_mk3 -lstar_conf_api_mk2 -lstar_conf_api_router -lstar-api -lrmap_packet_library

conf_router_SOURCES = test_static_routing.c utility.c
conf_router_LDADD  =  -lstar_conf_api_brick_mk3 -lstar_conf_api_mk2 -lstar_conf_api_router -lstar-api -lrmap_packet_library

receiv_SOURCES = test_receiv.c utility.c
receiv_LDADD  =  -lstar_conf_api_brick_mk3 -lstar_conf_api_mk2 -lstar_conf_api_router -lstar-api -lrmap_packet_library

timecode_SOURCES = test_timecode.c utility.c
timecode_LDADD  = -lpthread -lstar_conf_api_brick_mk3 -lstar_conf_api_mk2 -lstar_conf_api_router -lstar-api -lrmap_packet_library
