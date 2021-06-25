#include "GameObject.h"

GameObject::~GameObject()
{
}

void GameObject::Init()
{
	transform.getMatrix(model);
	prevTransform = transform;
}

void GameObject::Update()
{
	if (transform != prevTransform)
	{

		transform.getMatrix(model);
		prevTransform = transform;
	}

}

void GameObject::Destroy()
{
	mesh = nullptr;
	image = nullptr;
	for (auto& buffer : uniformBuffers)
	{
		buffer.Destroy();
	}
}
