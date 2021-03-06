﻿#include "stdafx.h"
#include <fstream>
#include "gmgamepackagehandler.h"
#include "foundation/utilities/tools.h"
#include "foundation/gamemachine.h"
#include "foundation/gmasync.h"
#include "gmgamepackage.h"

#define PKD(d) GMGamePackage::Data* d = gamePackage()->gamePackageData();

GMDefaultGamePackageHandler::GMDefaultGamePackageHandler(GMGamePackage* pk)
	: m_pk(pk)
{

}

bool GMDefaultGamePackageHandler::readFileFromPath(const GMString& path, REF GMBuffer* buffer)
{
	std::ifstream file;
	std::string p = path.toStdString();
	file.open(p, std::ios::in | std::ios::binary | std::ios::ate);
	if (file.good())
	{
		GMint32 size = file.tellg();
		if (size == -1)
		{
			gm_warning(gm_dbg_wrap("cannot read file from path: {0}"), path);
			return false;
		}

		buffer->size = size;
		buffer->buffer = new GMbyte[buffer->size];
		buffer->needRelease = true;

		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char*>(buffer->buffer), size);
		file.close();
		return true;
	}
	else
	{
		gm_warning(gm_dbg_wrap("cannot read file from path: {0}"), path);
	}
	return false;
}

void GMDefaultGamePackageHandler::beginReadFileFromPath(const GMString& path, GMAsyncCallback callback, OUT GMAsyncResult** ar)
{
	auto f = [this, path, callback](GMAsyncResult* r) {
		this->readFileFromPath(path, reinterpret_cast<GMBuffer*>(r->state()));
		r->setComplete();
		callback(r);
	};

	GMAsyncResult* asyncResult = new GMAsyncResult();
	GMFuture<void> future = GMAsync::async(GMAsync::Async, f, asyncResult);
	asyncResult->setFuture(std::move(future));
	GM_ASSERT(ar);
	(*ar) = asyncResult;
}

void GMDefaultGamePackageHandler::init()
{
}

GMString GMDefaultGamePackageHandler::pathRoot(GMPackageIndex index)
{
	PKD(d);

	switch (index)
	{
	case GMPackageIndex::Maps:
		return d->packagePath + L"maps/";
	case GMPackageIndex::Shaders:
		return d->packagePath + L"shaders/";
	case GMPackageIndex::TexShaders:
		return d->packagePath + L"texshaders/";
	case GMPackageIndex::Textures:
		return d->packagePath + L"textures/";
	case GMPackageIndex::Models:
		return d->packagePath + L"models/";
	case GMPackageIndex::Audio:
		return d->packagePath + L"audio/";
	case GMPackageIndex::Particle:
		return d->packagePath + L"particles/";
	case GMPackageIndex::Scripts:
		return d->packagePath + L"scripts/";
	case GMPackageIndex::Fonts:
		return d->packagePath + L"fonts/";
	default:
		GM_ASSERT(false);
		break;
	}
	return "";
}

GMGamePackage* GMDefaultGamePackageHandler::gamePackage()
{
	return m_pk;
}

Vector<GMString> GMDefaultGamePackageHandler::getAllFiles(const GMString& directory)
{
	return GMPath::getAllFiles(directory, true);
}

#define CHECK(err) if (err != UNZ_OK) return false

GMZipGamePackageHandler::GMZipGamePackageHandler(GMGamePackage* pk)
	: GMDefaultGamePackageHandler(pk)
	, m_uf(nullptr)
{
}

void GMZipGamePackageHandler::init()
{
	PKD(d);
	if (!loadZip())
	{
		gm_error(gm_dbg_wrap("invalid package file {0}"), d->packagePath);
		return;
	}
	Base::init();
}

GMZipGamePackageHandler::~GMZipGamePackageHandler()
{
	releaseUnzFile();
	releaseBuffers();
}

bool GMZipGamePackageHandler::readFileFromPath(const GMString& path, REF GMBuffer* buffer)
{
	if (m_buffers.find(path) != m_buffers.end())
	{
		ZipBuffer* buf = m_buffers[path];
		buffer->needRelease = false;
		buffer->buffer = buf->buffer;
		buffer->size = buf->size;
		return true;
	}

	GMString r = toRelativePath(path);
	if (m_buffers.find(toRelativePath(r)) != m_buffers.end())
	{
		ZipBuffer* buf = m_buffers[r];
		buffer->needRelease = false;
		buffer->buffer = buf->buffer;
		buffer->size = buf->size;
		return true;
	}

	gm_warning(gm_dbg_wrap("cannot find path {0} "), GMString(path));
	return false;
}

bool GMZipGamePackageHandler::loadZip()
{
	const GMuint32 bufSize = 4096;

	PKD(d);
	releaseUnzFile();

	m_uf = unzOpen64(d->packagePath.toStdString().c_str());
	unz_global_info64 gi;
	GMint32 err = unzGetGlobalInfo64(m_uf, &gi);
	CHECK(err);

	for (GMint32 i = 0; i < gi.number_entry; i++)
	{
		while (true)
		{
			char filename[FILENAME_MAX];
			unz_file_info64 file_info;
			err = unzGetCurrentFileInfo64(m_uf, &file_info, filename, sizeof(filename), NULL, 0, NULL, 0);
			CHECK(err);

			err = unzOpenCurrentFilePassword(m_uf, nullptr);
			CHECK(err);

#if GM_WINDOWS
			// 跳过文件夹
			if (file_info.external_fa & FILE_ATTRIBUTE_DIRECTORY)
				break;
#endif

			GMbyte* buffer = new GMbyte[file_info.uncompressed_size];
			GMbyte* ptr = buffer;
			do
			{
				err = unzReadCurrentFile(m_uf, ptr, bufSize);
				if (err < 0)
					return false;
				if (err > 0)
					ptr += err;
			} while (err > 0);
			GM_ASSERT(m_buffers.find(filename) == m_buffers.end());

			ZipBuffer* buf = new ZipBuffer();
			buf->buffer = buffer;
			buf->size = file_info.uncompressed_size;
			m_buffers[filename] = buf;

			break;
		}
		if ((i + 1) < gi.number_entry)
		{
			err = unzGoToNextFile(m_uf);
			CHECK(err);
		}
	}

	releaseUnzFile();
	return true;
}

void GMZipGamePackageHandler::releaseUnzFile()
{
	if (m_uf)
	{
		unzClose(m_uf);
		m_uf = nullptr;
	}
}

void GMZipGamePackageHandler::releaseBuffers()
{
	for (auto iter = m_buffers.begin(); iter != m_buffers.end(); iter++)
	{
		delete (*iter).second;
	}
}

GMString GMZipGamePackageHandler::toRelativePath(const GMString& in)
{
	Deque<std::wstring> deque;

	auto pushToStack = [&deque](std::wstring&& str) {
		if (str == L".")
			return;
		if (str == L"..")
		{
			deque.pop_back();
			return;
		}
		deque.push_back(str);
	};

	size_t pos1 = 0, pos2 = 0;
	const std::wstring& stdIn = in.toStdWString();
	for (; pos2 < stdIn.length(); pos2++)
	{
		if (stdIn[pos2] == L'\\' || stdIn[pos2] == L'/')
		{
			pushToStack(stdIn.substr(pos1, pos2 - pos1));
			pos1 = ++pos2;
		}
	}
	pushToStack(stdIn.substr(pos1, pos2 - pos1));

	GMString result("");
	while (!deque.empty())
	{
		result += deque.front();
		if (deque.size() > 1)
			result += "/";
		deque.pop_front();
	}
	return result;
}

Vector<GMString> GMZipGamePackageHandler::getAllFiles(const GMString& directory)
{
	Vector<GMString> result;
	GMString d = directory;
	for (auto& buffer : m_buffers)
	{
		if (buffer.first.toStdWString().compare(0, d.length(), d.toStdWString()) == 0)
		{
			result.push_back(buffer.first);
		}
	}
	return result;
}

GMString GMZipGamePackageHandler::pathRoot(GMPackageIndex index)
{
	PKD(d);

	switch (index)
	{
	case GMPackageIndex::Maps:
		return L"maps/";
	case GMPackageIndex::Shaders:
		return L"shaders/";
	case GMPackageIndex::TexShaders:
		return L"texshaders/";
	case GMPackageIndex::Textures:
		return L"textures/";
	case GMPackageIndex::Models:
		return L"models/";
	case GMPackageIndex::Audio:
		return L"audio/";
	case GMPackageIndex::Particle:
		return L"particles/";
	case GMPackageIndex::Scripts:
		return L"scripts/";
	case GMPackageIndex::Fonts:
		return L"fonts/";
	default:
		GM_ASSERT(false);
		break;
	}
	return L"";
}
