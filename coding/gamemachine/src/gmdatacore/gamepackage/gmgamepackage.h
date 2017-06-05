﻿#ifndef __GAMEPACKAGE_H__
#define __GAMEPACKAGE_H__
#include "common.h"
#include <string>
#include "foundation/utilities/utilities.h"
#include "foundation/vector.h"
BEGIN_NS

enum PackageIndex
{
	PI_MAPS,
	PI_SHADERS,
	PI_TEXSHADERS,
	PI_TEXTURES,
	PI_MODELS,
	PI_SOUNDS,
};

class GMBSPGameWorld;
struct IGamePackageHandler
{
	virtual ~IGamePackageHandler() {}
	virtual void init() = 0;
	virtual bool readFileFromPath(const char* path, REF GMBuffer* buffer) = 0;
	virtual std::string pathRoot(PackageIndex index) = 0;
	virtual AlignedVector<std::string> getAllFiles(const char* directory) = 0;
};

GM_PRIVATE_OBJECT(GMGamePackage)
{
	std::string packagePath;
	AutoPtr<IGamePackageHandler> handler;
	IFactory* factory;
};

class GMBSPGameWorld;
class GMGamePackage : public GMObject
{
	DECLARE_PRIVATE(GMGamePackage)

	friend class GameMachine;

	GMGamePackage(IFactory* factory);

public:
	Data* gamePackageData();
	void loadPackage(const char* path);
	void createBSPGameWorld(const char* map, OUT GMBSPGameWorld** gameWorld);
	bool readFile(PackageIndex index, const char* filename, REF GMBuffer* buffer, REF std::string* fullFilename = nullptr);
	AlignedVector<std::string> getAllFiles(const char* directory);

public:
	std::string pathOf(PackageIndex index, const char* filename);
	// 一般情况下，建议用readFile代替readFileFromPath，除非是想指定特殊的路径
	bool readFileFromPath(const char* path, REF GMBuffer* buffer);
};

END_NS
#endif