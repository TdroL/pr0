#pragma once

#include <app/comp/projection.hpp>

#include <core/event.hpp>

struct ProjectionChangedEvent : public event::Event<ProjectionChangedEvent>
{
	comp::Projection projection;
	const int width;
	const int height;
	ProjectionChangedEvent(comp::Projection projection, int width, int height) : projection{projection}, width{width}, height{height} {}
};