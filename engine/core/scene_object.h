#pragma once

#include <memory>

namespace engine::core
{

class SceneObjectLifetime final
{
};

using SceneObjectLifetimeToken = std::weak_ptr<const SceneObjectLifetime>;

class SceneObject
{
public:
    SceneObject() = default;
    virtual ~SceneObject() = default;

    SceneObject(const SceneObject&) = delete;
    SceneObject& operator=(const SceneObject&) = delete;
    SceneObject(SceneObject&&) = delete;
    SceneObject& operator=(SceneObject&&) = delete;

    virtual void reset() noexcept
    {
        _visible = true;
        _active = true;
        _destroyed = false;
    }

    void destroy() noexcept
    {
        if (_destroyed)
            return;

        _destroyed = true;
        on_destroyed();
    }
    [[nodiscard]] bool is_destroyed() const noexcept { return _destroyed; }
    [[nodiscard]] SceneObjectLifetimeToken lifetime_token() const noexcept
    {
        return _lifetime;
    }

    void set_visible(bool visible) noexcept { _visible = visible; }
    [[nodiscard]] bool is_visible() const noexcept { return _visible; }

    void set_active(bool active) noexcept { _active = active; }
    [[nodiscard]] bool is_active() const noexcept { return _active; }

    [[nodiscard]] virtual bool update_when_paused() const { return false; }
    [[nodiscard]] virtual bool receive_input_when_paused() const { return false; }

protected:
    virtual void on_destroyed() noexcept {}

private:
    std::shared_ptr<const SceneObjectLifetime> _lifetime =
        std::make_shared<SceneObjectLifetime>();
    bool _destroyed = false;
    bool _visible = true;
    bool _active = true;
};
}
