#pragma once

#include "Mesh.h"
#include "Image.h"
#include "Device.h"

#include <string>
#include <map>
#include <memory>

class Resources
{
public:
	Resources() = default;
	~Resources();

	void Init(Device* _devices);

	void Destroy();


	void LoadMesh(const std::string& name);

	void LoadImage(const std::string& name);

	std::map<std::string, std::unique_ptr<Mesh>> meshes;
	std::map<std::string, std::unique_ptr<Image>> images;

	Device* devices;

};

