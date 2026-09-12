#pragma once

#include "../../engine/scene/scene.h"
#include "../../engine/core/geometry/vector2.h"
#include "../../engine/core/scene_object_observer.h"
#include "../characters/enemy.h"
#include "../characters/player_character.h"
#include "../generator/enemy_generator.h"
#include "../map/dungeon_room.h"
#include "room_tile_collision_world.h"

#include <SDL.h>

class RoomScene final : public engine::scene::Scene
{
public:
    RoomScene() = default;
    ~RoomScene() override = default;

    void on_enter() override;
    void on_exit() override;
    void reset() override;

    void on_update(double delta) override;
    void on_render(SDL_Renderer *renderer) override;
    void on_input(const engine::input::InputSnapshot &input, const std::vector<engine::input::InputEvent> &events) override;

    // imgui debug
    void on_imgui() override;

private:
    engine::core::Vector2 get_shot_direction(int pointer_x, int pointer_y);

    void build_room();
    void spawn_player();
    void generate_enemies(EnemyType type, std::size_t count);

    engine::core::SceneObjectObserver<PlayerCharacter> _player;
    engine::core::SceneObjectObserver<DungeonRoom> _room;
    RoomTileCollisionWorld _collision_world;

    EnemyGenerator _enemy_generator;
};
