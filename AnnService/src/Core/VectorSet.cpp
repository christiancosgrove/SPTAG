// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "inc/Core/VectorSet.h"
#include "inc/Core/Common/CommonUtils.h"
#include "inc/Core/Common/DistanceUtils.h"
using namespace SPTAG;

VectorSet::VectorSet()
{
}


VectorSet::~VectorSet()
{
}


BasicVectorSet::BasicVectorSet(const ByteArray& p_bytesArray,
                               VectorValueType p_valueType,
                               DimensionType p_dimension,
                               SizeType p_vectorCount)
    : m_data(p_bytesArray),
      m_valueType(p_valueType),
      m_dimension(p_dimension),
      m_vectorCount(p_vectorCount),
      m_perVectorDataSize(static_cast<SizeType>(p_dimension * GetValueTypeSize(p_valueType)))
{
}


BasicVectorSet::~BasicVectorSet()
{
}


VectorValueType
BasicVectorSet::GetValueType() const
{
    return m_valueType;
}


void*
BasicVectorSet::GetVector(SizeType p_vectorID) const
{
    if (p_vectorID < 0 || p_vectorID >= m_vectorCount)
    {
        return nullptr;
    }

    return reinterpret_cast<void*>(m_data.Data() + ((size_t)p_vectorID) * m_perVectorDataSize);
}

void*
BasicVectorSet::GetData() const
{
    return reinterpret_cast<void*>(m_data.Data());
}

DimensionType
BasicVectorSet::Dimension() const
{
    return m_dimension;
}


SizeType
BasicVectorSet::Count() const
{
    return m_vectorCount;
}


bool
BasicVectorSet::Available() const
{
    return m_data.Data() != nullptr;
}


ErrorCode 
BasicVectorSet::Save(const std::string& p_vectorFile) const
{
    auto fp = SPTAG::f_createIO();
    if (fp == nullptr || !fp->Initialize(p_vectorFile.c_str(), std::ios::binary | std::ios::out)) return ErrorCode::FailedOpenFile;

    IOBINARY(fp, WriteBinary, sizeof(SizeType), (char*)&m_vectorCount);
    IOBINARY(fp, WriteBinary, sizeof(DimensionType), (char*)&m_dimension);
    IOBINARY(fp, WriteBinary, m_data.Length(), (char*)m_data.Data());
    return ErrorCode::Success;
}


SizeType BasicVectorSet::PerVectorDataSize() const 
{
    return (SizeType)m_perVectorDataSize;
}


void
BasicVectorSet::Normalize(int p_threads) 
{
    if (!SPTAG::COMMON::DistanceUtils::Quantizer)
    {
        switch (m_valueType)
        {
#define DefineVectorValueType(Name, Type) \
case SPTAG::VectorValueType::Name: \
SPTAG::COMMON::Utils::BatchNormalize<Type>(reinterpret_cast<Type *>(m_data.Data()), m_vectorCount, m_dimension, SPTAG::COMMON::Utils::GetBase<Type>(), p_threads); \
break; \

#include "inc/Core/DefinitionList.h"
#undef DefineVectorValueType
        default:
            break;
        }
    }
}