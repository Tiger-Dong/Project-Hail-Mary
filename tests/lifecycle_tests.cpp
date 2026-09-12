#include "../engine/core/game_object.h"
#include "../engine/core/scene_object_observer.h"
#include "../engine/resources/audio/audio_manager.h"
#include "../engine/resources/font/font_manager.h"
#include "../engine/resources/texture/texture_manager.h"
#include "../engine/scene/scene.h"
#include "../gameplay/combat/projectile_manager.h"
#include "../gameplay/combat/projectile_service.h"
#include "../gameplay/scene/room_scene.h"

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include <cassert>
#include <cstdlib>
#include <memory>

namespace
{

class TestScene final : public engine::scene::Scene
{
public:
    void on_enter() override {}
    void on_exit() override { destroy_all_scene_objects(); }
    void reset() override { destroy_all_scene_objects(); }

    engine::physics::PhysicsManager& physics() noexcept { return physics_manager(); }
};

class TrackedObject final : public engine::core::GameObject
{
public:
    explicit TrackedObject(int& destroy_count)
        : engine::core::GameObject(engine::core::DepthLayer::Item),
          _destroy_count(destroy_count)
    {
    }

private:
    void on_destroyed() noexcept override { ++_destroy_count; }

    int& _destroy_count;
};

}

int main()
{
    int destroy_count = 0;
    auto observed_object = std::make_unique<TrackedObject>(destroy_count);
    engine::core::SceneObjectObserver<TrackedObject> observer(observed_object.get());
    assert(observer.get() == observed_object.get());

    engine::core::SceneObject* base_object = observed_object.get();
    base_object->destroy();
    base_object->destroy();
    assert(destroy_count == 1);
    assert(observer.get() == nullptr);

    observed_object->reset();
    assert(observer.get() == observed_object.get());
    observed_object.reset();
    assert(observer.get() == nullptr);

    TestScene scene;
    int scene_destroy_count = 0;
    assert(scene.add_object(std::make_unique<TrackedObject>(scene_destroy_count)));
    scene.destroy_all_scene_objects();
    assert(scene_destroy_count == 1);

    ProjectileManager::instance()->bind_scene(scene, scene.physics());
    auto source = std::make_unique<TrackedObject>(destroy_count);
    ProjectileFireRequest request;
    request.source = source.get();
    request.shots.emplace_back();
    request.shots.front().spawn_delay_sec = 10.0f;
    PROJECTILE_SERVICE->request_fire(std::move(request));
    assert(ProjectileManager::instance()->scheduled_projectile_count() == 1);

    source.reset();
    ProjectileManager::instance()->update(0.1);
    assert(ProjectileManager::instance()->scheduled_projectile_count() == 0);
    ProjectileManager::instance()->unbind_scene();

    RoomScene room;
    room.on_enter();
    auto reset_source = std::make_unique<TrackedObject>(destroy_count);
    ProjectileFireRequest reset_request;
    reset_request.source = reset_source.get();
    reset_request.shots.emplace_back();
    reset_request.shots.front().spawn_delay_sec = 10.0f;
    PROJECTILE_SERVICE->request_fire(std::move(reset_request));
    assert(ProjectileManager::instance()->scheduled_projectile_count() == 1);
    room.reset();
    assert(ProjectileManager::instance()->scheduled_projectile_count() == 0);
    for (int reset_index = 0; reset_index < 9; ++reset_index)
        room.reset();
    room.on_exit();

    if (std::getenv("HAIL_SKIP_NATIVE_RESOURCE_TESTS"))
        return 0;

    SDL_setenv("SDL_AUDIODRIVER", "dummy", 1);
    assert(SDL_Init(SDL_INIT_AUDIO) == 0);

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(
        0, 4, 4, 32, SDL_PIXELFORMAT_RGBA32);
    assert(surface);
    SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(surface);
    assert(renderer);
    SDL_Texture* first = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 2, 2);
    SDL_Texture* second = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 2, 2);
    assert(first && second);

    engine::resources::TextureManager textures;
    assert(textures.store_texture("replaceable", first));
    assert(textures.store_texture("replaceable", second));
    assert(textures.find_texture("replaceable") == second);
    textures.clear();
    assert(textures.find_texture("replaceable") == nullptr);

    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(surface);

    assert(TTF_Init() == 0);
    {
        engine::resources::FontManager fonts;
        assert(fonts.load_font(
            "test", "assets/fonts/fusion-pixel-10px-proportional-latin.ttf", 16));
        assert(fonts.find_font("test"));
        fonts.clear();
        assert(fonts.find_font("test") == nullptr);
    }
    TTF_Quit();

    (void)Mix_Init(0);
    assert(Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) == 0);
    {
        engine::resources::AudioManager audio;
        assert(audio.load_sound("test", "assets/audio/test.wav"));
        assert(audio.find_sound("test"));
        audio.clear();
        assert(audio.find_sound("test") == nullptr);
    }
    Mix_CloseAudio();
    Mix_Quit();

    SDL_Quit();
    return 0;
}
