#pragma once

#include <engine/core/engine_state.hpp>

namespace tests::common::srk
{

inline constexpr engine::core::UID BND_N = engine::core::make_uid(
    engine::core::UIDDomain::INFRASTRUCTURE, engine::core::UIDKind::BOUNDARY_NODE, 1, 1);
inline constexpr engine::core::UID BND_S = engine::core::make_uid(
    engine::core::UIDDomain::INFRASTRUCTURE, engine::core::UIDKind::BOUNDARY_NODE, 1, 2);
inline constexpr engine::core::UID TOR_A = engine::core::make_uid(
    engine::core::UIDDomain::INFRASTRUCTURE, engine::core::UIDKind::TRACK_SECTION, 1, 1);
inline constexpr engine::core::UID TOR_B = engine::core::make_uid(
    engine::core::UIDDomain::INFRASTRUCTURE, engine::core::UIDKind::TRACK_SECTION, 1, 2);
inline constexpr engine::core::UID ZWR1 = engine::core::make_uid(
    engine::core::UIDDomain::INFRASTRUCTURE, engine::core::UIDKind::SWITCH, 1, 1);
inline constexpr engine::core::UID SEM_W = engine::core::make_uid(
    engine::core::UIDDomain::INFRASTRUCTURE, engine::core::UIDKind::SIGNAL, 1, 1);
inline constexpr engine::core::UID SEM_E = engine::core::make_uid(
    engine::core::UIDDomain::INFRASTRUCTURE, engine::core::UIDKind::SIGNAL, 1, 2);
inline constexpr engine::core::UID SIG1 = SEM_W;
inline constexpr engine::core::UID WK1 = engine::core::make_uid(
    engine::core::UIDDomain::INFRASTRUCTURE, engine::core::UIDKind::DERAILER, 1, 1);
inline constexpr engine::core::UID BL1 = engine::core::make_uid(
    engine::core::UIDDomain::INFRASTRUCTURE, engine::core::UIDKind::BLOCK_SECTION, 1, 1);

inline constexpr engine::core::UID IT_A_N = engine::core::make_uid(
    engine::core::UIDDomain::INFRASTRUCTURE, engine::core::UIDKind::AXLE_COUNTER, 1, 1);
inline constexpr engine::core::UID IZ_A_S = engine::core::make_uid(
    engine::core::UIDDomain::INFRASTRUCTURE, engine::core::UIDKind::AXLE_COUNTER, 1, 2);
inline constexpr engine::core::UID IZ_B_N = engine::core::make_uid(
    engine::core::UIDDomain::INFRASTRUCTURE, engine::core::UIDKind::AXLE_COUNTER, 1, 3);
inline constexpr engine::core::UID IT_B_S = engine::core::make_uid(
    engine::core::UIDDomain::INFRASTRUCTURE, engine::core::UIDKind::AXLE_COUNTER, 1, 4);
inline constexpr engine::core::UID IZ_DIV = engine::core::make_uid(
    engine::core::UIDDomain::INFRASTRUCTURE, engine::core::UIDKind::AXLE_COUNTER, 1, 5);

inline constexpr engine::core::UID STA1 = engine::core::make_uid(
    engine::core::UIDDomain::INFRASTRUCTURE, engine::core::UIDKind::STATION, 1, 1);
inline constexpr engine::core::UID STA_NGR = engine::core::make_uid(
    engine::core::UIDDomain::INFRASTRUCTURE, engine::core::UIDKind::STATION, 2, 1);

inline engine::core::EngineState make_linear_infra_state()
{
    using namespace engine::core;

    EngineState st;
    st.set_session_id("TEST");
    st.set_current_tick(1);

    BoundaryNode bn_n;
    bn_n.uid = BND_N;
    bn_n.pid = "BND-N";
    bn_n.station_uid = STA1;

    BoundaryNode bn_s;
    bn_s.uid = BND_S;
    bn_s.pid = "BND-S";
    bn_s.station_uid = STA1;

    st.insert_boundary_node(bn_n);
    st.insert_boundary_node(bn_s);

    Signal sem_w;
    sem_w.uid = SEM_W;
    sem_w.pid = "Wp1";
    sem_w.station_uid = STA1;
    sem_w.type = Signal::Type::ENTRY;
    sem_w.governs_section_uid = TOR_A;
    sem_w.current_aspect = SignalAspect::S1_STOP;
    st.insert_signal(sem_w);

    Signal sem_e;
    sem_e.uid = SEM_E;
    sem_e.pid = "Wy1";
    sem_e.station_uid = STA1;
    sem_e.type = Signal::Type::DEPARTURE;
    sem_e.governs_section_uid = TOR_B;
    sem_e.current_aspect = SignalAspect::S1_STOP;
    st.insert_signal(sem_e);

    TrackSection ta;
    ta.uid = TOR_A;
    ta.pid = "tor_a";
    ta.station_uid = STA1;
    ta.side_a.neighbor_uid = BND_N;
    ta.side_a.counter_uid = IT_A_N;
    ta.side_a.counter_kind = TrackPort::CounterKind::IT;
    ta.side_a.signal_uids = {SEM_W};
    ta.side_b.neighbor_uid = ZWR1;
    ta.side_b.counter_uid = IZ_A_S;
    ta.side_b.counter_kind = TrackPort::CounterKind::IZ;
    ta.occupancy = TrackOccupancy::FREE;
    st.insert_track_section(ta);

    TrackSection tb;
    tb.uid = TOR_B;
    tb.pid = "tor_b";
    tb.station_uid = STA1;
    tb.side_a.neighbor_uid = ZWR1;
    tb.side_a.counter_uid = IZ_B_N;
    tb.side_a.counter_kind = TrackPort::CounterKind::IZ;
    tb.side_b.neighbor_uid = BND_S;
    tb.side_b.counter_uid = IT_B_S;
    tb.side_b.counter_kind = TrackPort::CounterKind::IT;
    tb.side_b.signal_uids = {SEM_E};
    tb.occupancy = TrackOccupancy::FREE;
    st.insert_track_section(tb);

    Switch sw1;
    sw1.uid = ZWR1;
    sw1.pid = "zwr1";
    sw1.station_uid = STA1;
    sw1.type_id = "DVT-GLB-ZWR-EEA4-0000002";
    sw1.trunk.neighbor_uid = TOR_A;
    sw1.trunk.iz_uid = IZ_A_S;
    sw1.straight.neighbor_uid = TOR_B;
    sw1.straight.iz_uid = IZ_B_N;
    sw1.divergent.neighbor_uid = BND_S;
    sw1.divergent.iz_uid = IZ_DIV;
    sw1.position = SwitchPosition::STRAIGHT;
    sw1.occupancy = TrackOccupancy::FREE;
    st.insert_switch(sw1);

    Derailer wk;
    wk.uid = WK1;
    wk.pid = "wk1";
    wk.station_uid = STA1;
    wk.type_id = "DVT-GLB-WK-0000004";
    wk.guards_section_uid = TOR_A;
    wk.state = DerailerState::LOCKED;
    st.insert_derailer(wk);

    return st;
}

inline engine::core::EngineState make_block_only_state(
    engine::core::BlockDirectionState dir = engine::core::BlockDirectionState::NEUTRAL,
    engine::core::BlockSectionState state = engine::core::BlockSectionState::CLOSED,
    int axle_count = 0)
{
    using namespace engine::core;

    EngineState st;
    st.set_session_id("TEST");
    st.set_current_tick(1);

    BlockSection bs;
    bs.uid = BL1;
    bs.pid = "bl1";
    bs.station_uid = STA1;
    bs.neighbor_station_uid = STA_NGR;
    bs.direction = dir;
    bs.state = state;
    bs.axle_count = axle_count;
    st.insert_block_section(bs);

    return st;
}

inline engine::core::EngineState make_linear_infra_state_with_block(
    engine::core::BlockDirectionState dir = engine::core::BlockDirectionState::NEUTRAL,
    engine::core::BlockSectionState state = engine::core::BlockSectionState::CLOSED,
    int axle_count = 0)
{
    auto st = make_linear_infra_state();

    engine::core::BlockSection bs;
    bs.uid = BL1;
    bs.pid = "bl1";
    bs.station_uid = STA1;
    bs.neighbor_station_uid = STA_NGR;
    bs.direction = dir;
    bs.state = state;
    bs.axle_count = axle_count;
    st.insert_block_section(bs);

    return st;
}

inline engine::core::EngineState make_signal_only_state()
{
    using namespace engine::core;

    EngineState st;
    st.set_session_id("TEST");
    st.set_current_tick(1);

    Signal sig;
    sig.uid = SIG1;
    sig.pid = "A";
    sig.station_uid = STA1;
    sig.type = Signal::Type::ENTRY;
    sig.current_aspect = SignalAspect::S1_STOP;
    st.insert_signal(sig);

    return st;
}

}  // namespace tests::common::srk
