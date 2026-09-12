#include "room_scene.h"

#include "../../engine/physics/collision_manager.h"
#include "../../engine/input/input_state.h"

#include "../combat/bullet.h"
#include "../combat/attack_info.h"
#include "../combat/projectile.h"

#include "../combat/projectile_manager.h"

#include "../map/dungeon_room.h"
#include "../../thirdparty/imgui/imgui.h"

#include <memory>
#include <vector>

void RoomScene::on_enter()
{
    _paused = false;

    build_room();
    spawn_player();
    generate_enemies(EnemyType::Skeleton, 3);
    generate_enemies(EnemyType::Slime, 3);
    generate_enemies(EnemyType::GoblinWitch, 2);
    generate_enemies(EnemyType::Wizard, 1);
    generate_enemies(EnemyType::SkeletonElite, 2);


    ProjectileManager::instance()->bind_scene(*this, physics_manager());
}

void RoomScene::on_update(double delta)
{
    this->engine::scene::Scene::on_update(delta);

    ProjectileManager::instance()->update(delta);

    PlayerCharacter* player = _player.get();
    if (player && !player->is_dead())
    {
        camera.follow(player->center().x, player->center().y, 1.0f);
    }
}

void RoomScene::on_render(SDL_Renderer *renderer)
{
    this->engine::scene::Scene::on_render(renderer);
}

void RoomScene::on_input(const engine::input::InputSnapshot &input, const std::vector<engine::input::InputEvent> &events)
{
    this->engine::scene::Scene::on_input(input, events);

    if (input.state.is_just_pressed(engine::input::InputAction::Reset))
    {
        reset();
        return;
    }

    PlayerCharacter* player = _player.get();
    if (!player || player->is_dead())
        return;

    if (input.state.is_just_pressed(engine::input::InputAction::Attack))
    {
        engine::core::Vector2 shot_direction(1.0f, 0.0f);
        if (input.has_pointer_position)
        {
            shot_direction = get_shot_direction(input.pointer_x, input.pointer_y);
        }

        // Get schedule of projectiles from wand attack and add to buffer
        player->create_projectile(shot_direction);
    }
}

// imgui debug
void RoomScene::on_imgui()
{
    // if (_player && !_player->is_destroyed())
    // {
    //     ImGui::Separator();
    //     _player->wand().debug_data().render_debugger();
    // }
}

void RoomScene::on_exit()
{
    ProjectileManager::instance()->unbind_scene();
    this->destroy_all_scene_objects();
    _collision_world.set_room(nullptr);
    _player.reset();
    _paused = false;
    _room.reset();
}

void RoomScene::reset()
{
    ProjectileManager::instance()->unbind_scene();
    this->destroy_all_scene_objects();
    _collision_world.set_room(nullptr);
    _player.reset();
    _paused = false;
    _room.reset();
    build_room();
    spawn_player();
    generate_enemies(EnemyType::Skeleton, 3);
    generate_enemies(EnemyType::Slime, 3);
    generate_enemies(EnemyType::GoblinWitch, 2);
    generate_enemies(EnemyType::Wizard, 1);
    generate_enemies(EnemyType::SkeletonElite, 2);
    ProjectileManager::instance()->bind_scene(*this, physics_manager());
}

engine::core::Vector2 RoomScene::get_shot_direction(int pointer_x, int pointer_y)
{
    const SDL_FPoint mouse_world = camera.screen_to_world(
        static_cast<float>(pointer_x),
        static_cast<float>(pointer_y));

    PlayerCharacter* player = _player.get();
    if (!player)
        return engine::core::Vector2(1.0f, 0.0f);

    engine::core::Vector2 aim_direction = engine::core::Vector2(mouse_world.x, mouse_world.y) - player->center();
    if (aim_direction.is_zero())
        aim_direction = engine::core::Vector2(1.0f, 0.0f);

    return aim_direction.normalized();
}

void RoomScene::build_room()
{
    if (_room)
        return;

    DungeonRoom* room = add_object(std::make_unique<DungeonRoom>());
    _room.reset(room);
    if (room)
    {
        _collision_world.set_room(room);
        physics_manager().set_collision_world(&_collision_world);
    }
}

void RoomScene::spawn_player()
{
    PlayerCharacter* player = _player.get();
    if (player && !player->is_dead())
        return;

    player = create_and_add_object<PlayerCharacter>(
        "player/protagonist",
        engine::core::Vector2(540.0f, 540.0f),
        engine::core::Vector2(64.0f, 64.0f),
        "fire.impact_radial");
    _player.reset(player);

    if (player)
    {
        player->set_move_speed(200.0f);
        physics_manager().register_body(player, player, player);
    }
}

void RoomScene::generate_enemies(EnemyType type, std::size_t count)
{
    DungeonRoom* room = _room.get();
    if (!room)
        return;

    const EnemyGenerationConfig config{
        .type = type,
        .count = count};

    std::vector<std::unique_ptr<Enemy>> generated_enemies =
        _enemy_generator.generate(*room, config);

    for (std::unique_ptr<Enemy> &enemy : generated_enemies)
    {
        Enemy *added_enemy = add_object(std::move(enemy));
        if (!added_enemy)
            continue;

        physics_manager().register_body(added_enemy, added_enemy, added_enemy);
    }
}
