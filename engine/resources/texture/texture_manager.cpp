#include "texture_manager.h"
#include "../../tools/logger.h"
#include <SDL_image.h>
#include <utility>

namespace engine::resources
{

TextureManager::~TextureManager()
{
	clear();
}

bool TextureManager::load_texture(SDL_Renderer* renderer, const std::string& key,std::filesystem::path file_path)
{
	if (!renderer)
	{
		ENGINE_LOG_WARN("resource","Load texture failed: renderer is empty.");

		return false;
	}

	SDL_Texture* texture = IMG_LoadTexture(renderer, file_path.string().c_str());

	return store_texture(key,texture);
}

bool TextureManager::store_texture(const std::string& key, SDL_Texture* texture)
{
	if (key.empty())
	{
		ENGINE_LOG_WARN("resource","Store texture failed: key is empty.");
		if (texture)
			SDL_DestroyTexture(texture);
		return false;
	}

	if (!texture)
	{
		ENGINE_LOG_WARN("resource","Store texture failed: texture is null: " << key);
		return false;
	}

	TexturePool::iterator iterator = _texture_pool.find(key);
	if (iterator != _texture_pool.end())
	{
		if (iterator->second != texture)
		{
			SDL_DestroyTexture(iterator->second);
			iterator->second = texture;
		}
		return true;
	}

	_texture_pool.emplace(key, texture);
	return true;
}

SDL_Texture* TextureManager::find_texture(const std::string_view& key)
{
	TexturePool::const_iterator iterator = _texture_pool.find(std::string(key));
	if (iterator == _texture_pool.end())
		return nullptr;

	return iterator->second;
}

void TextureManager::clear() noexcept
{
	for (const auto& [key, texture] : _texture_pool)
	{
		(void)key;
		if (texture)
			SDL_DestroyTexture(texture);
	}
	_texture_pool.clear();
}
}
