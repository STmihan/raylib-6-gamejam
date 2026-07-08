#include "logic/sim/simulation.h"

#include "data/sim/sim_config.h"
#include "data/space/hex.h"
#include "data/space/world_config.h"

namespace logic {

namespace {

void SpawnDemoEntity(Entity &entity, std::uint32_t id) {
    data::Vec2 extent = data::FieldExtentLogic();
    float lane = static_cast<float>(id % data::FieldCols) / static_cast<float>(data::FieldCols - 1);
    float speed = data::SpawnSpeedBase + data::SpawnSpeedStep * static_cast<float>(id % data::SpawnSpeedTiers);
    float direction = (id % 2 == 0) ? 1.0f : -1.0f;

    entity.active = true;
    entity.id = id;
    entity.position = { lane * extent.x,
                        extent.y * 0.5f + data::SpawnYOffsetStep * static_cast<float>(id % data::SpawnYSpread) };
    entity.velocity = { 0.0f, direction * speed };
}

void IntegrateBounce(Entity &entity, float dt) {
    data::Vec2 extent = data::FieldExtentLogic();
    entity.position = data::Add(entity.position, data::Scale(entity.velocity, dt));

    if (entity.position.y < 0.0f) {
        entity.position.y = 0.0f;
        entity.velocity.y = -entity.velocity.y;
    } else if (entity.position.y > extent.y) {
        entity.position.y = extent.y;
        entity.velocity.y = -entity.velocity.y;
    }
}

}

void Simulation::Init(GameState &state) {
    state.tick = 0;
    state.entityCount = data::DemoEntityCount;
    for (int i = 0; i < data::MaxEntities; i++) {
        state.entities[i].active = false;
        state.entities[i].id = 0;
        state.entities[i].position = { 0.0f, 0.0f };
        state.entities[i].velocity = { 0.0f, 0.0f };
    }
    for (int i = 0; i < data::DemoEntityCount; i++) {
        SpawnDemoEntity(state.entities[i], static_cast<std::uint32_t>(i));
    }
}

void Simulation::Step(GameState &state, float dt) {
    state.tick++;
    for (int i = 0; i < data::MaxEntities; i++) {
        if (!state.entities[i].active) continue;
        IntegrateBounce(state.entities[i], dt);
    }
}

}
