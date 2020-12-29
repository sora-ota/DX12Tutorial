#pragma once

//===============================================
//
// STL（インクルード周りどうするかはエンジン作るときに考える）
//
//===============================================
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <string>
#include <array>
#include <vector>
#include <stack>
#include <list>
#include <iterator>
#include <queue>
#include <algorithm>
#include <memory>
#include <random>
#include <fstream>
#include <iostream>
#include <sstream>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <future>
#include <filesystem>

//======================================
//
// 型
//
//======================================
#include <stdexcept>
using b8 = bool;
using s8 = int8_t;
using u8 = uint8_t;
using s16 = int16_t;
using u16 = uint16_t;
using s32 = int32_t;
using u32 = uint32_t;
using s64 = int64_t;
using u64 = uint64_t;

//======================================
//
// スマートポインター
//
//======================================
#include <wrl.h>

//ComPtr
using Microsoft::WRL::ComPtr;

//SharedPtr
template<class T>
using SPtr = std::shared_ptr<T>;

//WeakPtr
template<class T>
using WPtr = std::weak_ptr<T>;

//Unique_Ptr
template<class T>
using UPtr = std::unique_ptr<T>;

//スマートポインタ用メモリ確保
//SharedPtr
template<class T, class... Args>
inline SPtr<T> MakeSPtr(Args&&... args)
{
	return std::make_shared<T>(args...);
}
//UniquePtr
template<class T, class... Args>
inline UPtr<T> MakeUPtr(Args&&... args) {
	return std::make_unique<T>(args...);
}

//======================================
//
// 安全に解放する系のマクロ
//
//======================================
// 安全にRelease関数を実行する
template<class T>
void otSafeRelease(T*& p)
{
	if (p) {
		p->Release();
		p = nullptr;
	}
}

// 安全にDelete関数を実行する
template<class T>
void otSafeDelete(T*& p)
{
	if (p) {
		delete p;
		p = nullptr;
	}
}
