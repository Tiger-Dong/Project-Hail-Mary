#pragma once

#include "scene_object.h"

#include <cstddef>
#include <type_traits>

namespace engine::core
{

template <typename T>
class SceneObjectObserver
{
    static_assert(
        std::is_base_of_v<SceneObject, std::remove_const_t<T>>,
        "T must derive from SceneObject.");

public:
    SceneObjectObserver() = default;
    SceneObjectObserver(std::nullptr_t) noexcept {}
    explicit SceneObjectObserver(T* object) noexcept { reset(object); }

    void reset(T* object = nullptr) noexcept
    {
        _object = object;
        _lifetime = object ? object->lifetime_token() : SceneObjectLifetimeToken{};
    }

    [[nodiscard]] T* get() const noexcept
    {
        if (!_object || _lifetime.expired())
            return nullptr;

        return _object->is_destroyed() ? nullptr : _object;
    }

    [[nodiscard]] explicit operator bool() const noexcept { return get() != nullptr; }

private:
    T* _object = nullptr;
    SceneObjectLifetimeToken _lifetime;
};

}
