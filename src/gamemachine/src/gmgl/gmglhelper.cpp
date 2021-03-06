﻿#include "stdafx.h"
#include "gmglhelper.h"
#include "gmglshaderprogram.h"

bool GMGLHelper::loadShader(
	const IRenderContext* context,
	const GMGLShaderContent& forwardVertex,
	const GMGLShaderContent& forwardPixel,
	const GMGLShaderContent& deferredGeometryVertex,
	const GMGLShaderContent& deferredGeometryPixel,
	const GMGLShaderContent& deferredLightVertex,
	const GMGLShaderContent& deferredLightPixel,
	const GMGLShaderContent& filtersVertex,
	const GMGLShaderContent& filtersPixel
)
{
	GMGLShaderProgram* forward = new GMGLShaderProgram(context);
	GMGLShaderProgram* deferredGeometry = new GMGLShaderProgram(context);
	GMGLShaderProgram* deferredLight = new GMGLShaderProgram(context);
	GMGLShaderProgram* filter = new GMGLShaderProgram(context);

	{
		GMGLShaderInfo shadersInfo[] = {
			{ GMGLShaderInfo::toGLShaderType(gm::GMShaderType::Vertex), (const char*)forwardVertex.code.toStdString().c_str(), forwardVertex.path },
			{ GMGLShaderInfo::toGLShaderType(gm::GMShaderType::Pixel), (const char*)forwardPixel.code.toStdString().c_str(), forwardPixel.path },
		};
		forward->attachShader(shadersInfo[0]);
		forward->attachShader(shadersInfo[1]);
	}

	{
		GMGLShaderInfo shadersInfo[] = {
			{ GMGLShaderInfo::toGLShaderType(gm::GMShaderType::Vertex), (const char*)deferredGeometryVertex.code.toStdString().c_str(), deferredGeometryVertex.path },
			{ GMGLShaderInfo::toGLShaderType(gm::GMShaderType::Pixel), (const char*)deferredGeometryPixel.code.toStdString().c_str(), deferredGeometryPixel.path },
		};
		deferredGeometry->attachShader(shadersInfo[0]);
		deferredGeometry->attachShader(shadersInfo[1]);
	}

	{
		GMGLShaderInfo shadersInfo[] = {
			{ GMGLShaderInfo::toGLShaderType(gm::GMShaderType::Vertex), (const char*)deferredLightVertex.code.toStdString().c_str(), deferredLightVertex.path },
			{ GMGLShaderInfo::toGLShaderType(gm::GMShaderType::Pixel), (const char*)deferredLightPixel.code.toStdString().c_str(), deferredLightPixel.path },
		};
		deferredLight->attachShader(shadersInfo[0]);
		deferredLight->attachShader(shadersInfo[1]);
	}

	{
		GMGLShaderInfo shadersInfo[] = {
			{ GMGLShaderInfo::toGLShaderType(gm::GMShaderType::Vertex), (const char*)filtersVertex.code.toStdString().c_str(), filtersVertex.path },
			{ GMGLShaderInfo::toGLShaderType(gm::GMShaderType::Pixel), (const char*)filtersPixel.code.toStdString().c_str(), filtersPixel.path },
		};
		filter->attachShader(shadersInfo[0]);
		filter->attachShader(shadersInfo[1]);
	}

	bool result = forward->load() && deferredGeometry->load() && deferredLight->load() && filter->load();
	if (!result)
		return result;
	
	result = context->getEngine()->setInterface(gm::GameMachineInterfaceID::GLForwardShaderProgram, forward);
	if (!result)
		return result;

	result = context->getEngine()->setInterface(gm::GameMachineInterfaceID::GLDeferredShaderGeometryProgram, deferredGeometry);
	if (!result)
		return result;

	result = context->getEngine()->setInterface(gm::GameMachineInterfaceID::GLDeferredShaderLightProgram, deferredLight);
	if (!result)
		return result;

	result = context->getEngine()->setInterface(gm::GameMachineInterfaceID::GLFiltersShaderProgram, filter);
	return result;
}

bool GMGLHelper::loadShader(
	const IRenderContext* context,
	const GMString& forwardVertexFilePath,
	const GMString& forwardPixelFilePath,
	const GMString& deferredGeometryVertexFilePath,
	const GMString& deferredGeometryPixelFilePath,
	const GMString& deferredLightVertexFilePath,
	const GMString& deferredLightPixelFilePath,
	const GMString& filtersVertexFilePath,
	const GMString& filtersPixelFilePath
)
{
	GMGLShaderContent forwardVertex;
	GMGLShaderContent forwardPixel;
	GMGLShaderContent deferredGeometryVertex;
	GMGLShaderContent deferredGeometryPixel;
	GMGLShaderContent deferredLightVertex;
	GMGLShaderContent deferredLightPixel;
	GMGLShaderContent filtersVertex;
	GMGLShaderContent filtersPixel;

	gm::GMString vertPath, pixPath;
	{
		gm::GMBuffer vertBuf, pixBuf;
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, forwardVertexFilePath, &vertBuf, &vertPath);
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, forwardPixelFilePath, &pixBuf, &pixPath);
		vertBuf.convertToStringBuffer();
		pixBuf.convertToStringBuffer();

		forwardVertex.code = GMString(reinterpret_cast<const char*>(vertBuf.buffer));
		forwardVertex.path = vertPath;
		forwardPixel.code = GMString(reinterpret_cast<const char*>(pixBuf.buffer));
		forwardPixel.path = pixPath;
	}

	{
		gm::GMBuffer vertBuf, pixBuf;
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, deferredGeometryVertexFilePath, &vertBuf, &vertPath);
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, deferredGeometryPixelFilePath, &pixBuf, &pixPath);
		vertBuf.convertToStringBuffer();
		pixBuf.convertToStringBuffer();

		deferredGeometryVertex.code = GMString(reinterpret_cast<const char*>(vertBuf.buffer));
		deferredGeometryVertex.path = vertPath;
		deferredGeometryPixel.code = GMString(reinterpret_cast<const char*>(pixBuf.buffer));
		deferredGeometryPixel.path = pixPath;
	}

	{
		gm::GMBuffer vertBuf, pixBuf;
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, deferredLightVertexFilePath, &vertBuf, &vertPath);
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, deferredLightPixelFilePath, &pixBuf, &pixPath);
		vertBuf.convertToStringBuffer();
		pixBuf.convertToStringBuffer();

		deferredLightVertex.code = GMString(reinterpret_cast<const char*>(vertBuf.buffer));
		deferredLightVertex.path = vertPath;
		deferredLightPixel.code = GMString(reinterpret_cast<const char*>(pixBuf.buffer));
		deferredLightPixel.path = pixPath;
	}

	{
		gm::GMBuffer vertBuf, pixBuf;
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, filtersVertexFilePath, &vertBuf, &vertPath);
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, filtersPixelFilePath, &pixBuf, &pixPath);
		vertBuf.convertToStringBuffer();
		pixBuf.convertToStringBuffer();

		filtersVertex.code = GMString(reinterpret_cast<const char*>(vertBuf.buffer));
		filtersVertex.path = vertPath;
		filtersPixel.code = GMString(reinterpret_cast<const char*>(pixBuf.buffer));
		filtersPixel.path = pixPath;
	}

	return loadShader(
		context,
		forwardVertex,
		forwardPixel,
		deferredGeometryVertex,
		deferredGeometryPixel,
		deferredLightVertex,
		deferredLightPixel,
		filtersVertex,
		filtersPixel
	);
}