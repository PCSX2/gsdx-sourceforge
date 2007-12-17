#pragma once

#pragma pack(push, 1)

struct GSVector2
{
	union {struct {float x, y;}; struct {float r, g;};};
	struct GSVector2() {x = y = 0;}
	struct GSVector2(float x, float y) {this->x = x; this->y = y;}
	bool operator == (const GSVector2& v) const {return x == v.x && y == v.y;}
};

struct GSVector3
{
	union {struct {float x, y, z;}; struct {float r, g, b;};};
	struct GSVector3() {x = y = z = 0;}
	struct GSVector3(float x, float y, float z) {this->x = x; this->y = y; this->z = z;}
	bool operator == (const GSVector3& v) const {return x == v.x && y == v.y && z == v.z;}
};

struct GSVector4
{
	union {struct {float x, y, z, w;}; struct {float r, g, b, a;};};
	struct GSVector4() {x = y = z = w = 0;}
	struct GSVector4(float x, float y, float z = 0.5f, float w = 1.0f) {this->x = x; this->y = y; this->z = z; this->w = w;}
	bool operator == (const GSVector4& v) const {return x == v.x && y == v.y && z == v.z && w == v.w;}
};

#pragma pack(pop)
