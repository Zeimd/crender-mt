/*****************************************************************************
*
* eng-object.h
*
* By Jari Korkala 10/2011
*
*****************************************************************************/

#ifndef _ENG_OBJECT_H
#define _ENG_OBJECT_H

#include <ceng.h>

class EngineObject
{
protected:
	Ceng::VectorF4 position;

	Ceng::Quaternion rotation;
	Ceng::VectorF4 rotationBasis[3];

	Ceng::Matrix4 positionMatrix;
public:
	EngineObject();
	virtual ~EngineObject();

	virtual void RotateByDeltas(const Ceng::FLOAT32 xAngle,
								const Ceng::FLOAT32 yAngle,
								const Ceng::FLOAT32 zAngle);

	virtual const Ceng::Matrix4 GetRotationMatrix() const;
	virtual const Ceng::Matrix4 GetPositionMatrix();
	virtual const Ceng::Matrix4 GetFullTransformation();

	void SetWorldPosition(const Ceng::VectorF4 &newPos);
	
	virtual void MoveByDelta(const Ceng::VectorF4 &deltaPos);
	virtual void MoveByWorldDelta(const Ceng::VectorF4 &deltaPos);
};

struct ColorF4
{
	Ceng::FLOAT32 blue;
	Ceng::FLOAT32 green;
	Ceng::FLOAT32 red;
	Ceng::FLOAT32 alpha;
};

struct TexelF4
{
	Ceng::FLOAT32 u;
	Ceng::FLOAT32 v;
	Ceng::FLOAT32 s;
	Ceng::FLOAT32 t;
};

struct TexelF2
{
	Ceng::FLOAT32 u;
	Ceng::FLOAT32 v;
};

struct ProgVertexData
{
	Ceng::VectorF4 location;
//	Ceng::VectorF4 normal;
//	ColorF4 color;
	
	TexelF2 texcoord;
	TexelF2 padding;
//	Ceng::FLOAT32 colorMul;
//	Ceng::FLOAT32 testConst;
};

struct SurfaceData
{
	Ceng::INT32 vertexIndices[8];
};

class Mesh
{
public:
	std::vector<ProgVertexData> vertices;
	std::vector<SurfaceData> surfaces;

public:
	Mesh();
	~Mesh();
};

class Camera : public EngineObject
{
protected:

	Ceng::Quaternion reverseRotation;

	Ceng::VectorF4 defaultBasis[3];

public:
	Camera();
	virtual ~Camera();

	virtual void RotateByDeltas(const Ceng::FLOAT32 xAngle,
								const Ceng::FLOAT32 yAngle,
								const Ceng::FLOAT32 zAngle);

	virtual const Ceng::Matrix4 GetRotationMatrix() const;
	virtual const Ceng::Matrix4 GetPositionMatrix();
	virtual const Ceng::Matrix4 GetFullTransformation();
};

class Actor : public EngineObject
{
protected:
	Mesh *mesh;
public:
	Actor();
	virtual ~Actor();

	Ceng::CRESULT SetMesh(Mesh *meshPtr);
};

const Ceng::String ImageFormatToString(const Ceng::IMAGE_FORMAT::value format);

const Ceng::CRESULT CreateTexture2dFromFile(const Ceng::StringUtf8 &file, Ceng::Texture2dDesc &desc,
	Ceng::RenderDevice *renderDevice, Ceng::Texture2D **out_texture);

const Ceng::CRESULT CreateTexture2d_BMP(const Ceng::StringUtf8 &file, Ceng::Texture2dDesc &desc,
	Ceng::RenderDevice *renderDevice, Ceng::Texture2D **out_texture);

#endif