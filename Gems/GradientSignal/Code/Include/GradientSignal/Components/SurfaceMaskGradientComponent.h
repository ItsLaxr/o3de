/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <LmbrCentral/Dependency/DependencyMonitor.h>
#include <GradientSignal/GradientSampler.h>
#include <AzCore/Component/Component.h>
#include <GradientSignal/Ebuses/GradientRequestBus.h>
#include <GradientSignal/Ebuses/SurfaceMaskGradientRequestBus.h>
#include <SurfaceData/SurfaceDataTypes.h>
#include <SurfaceData/SurfaceDataSystemRequestBus.h>

namespace LmbrCentral
{
    template<typename, typename>
    class EditorWrappedComponentBase;
}

namespace GradientSignal
{
    class SurfaceMaskGradientConfig
        : public AZ::ComponentConfig
    {
    public:
        AZ_CLASS_ALLOCATOR(SurfaceMaskGradientConfig, AZ::SystemAllocator, 0);
        AZ_RTTI(SurfaceMaskGradientConfig, "{E59D0A4C-BA3D-4288-B409-A00B7D5566AA}", AZ::ComponentConfig);
        static void Reflect(AZ::ReflectContext* context);
        SurfaceData::SurfaceTagVector m_surfaceTagList;

        size_t GetNumTags() const;
        AZ::Crc32 GetTag(int tagIndex) const;
        void RemoveTag(int tagIndex);
        void AddTag(AZStd::string tag);
    };

    static const AZ::Uuid SurfaceMaskGradientComponentTypeId = "{4661F063-7126-4BE1-886F-5A6FFC6DAC71}";

    /**
    * calculates a gradient value based on percent contribution from surface tags
    */      
    class SurfaceMaskGradientComponent
        : public AZ::Component
        , private GradientRequestBus::Handler
        , private SurfaceMaskGradientRequestBus::Handler
    {
    public:
        template<typename, typename> friend class LmbrCentral::EditorWrappedComponentBase;
        AZ_COMPONENT(SurfaceMaskGradientComponent, SurfaceMaskGradientComponentTypeId);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& services);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& services);
        static void Reflect(AZ::ReflectContext* context);

        SurfaceMaskGradientComponent(const SurfaceMaskGradientConfig& configuration);
        SurfaceMaskGradientComponent() = default;
        ~SurfaceMaskGradientComponent() = default;

        //////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;
        bool ReadInConfig(const AZ::ComponentConfig* baseConfig) override;
        bool WriteOutConfig(AZ::ComponentConfig* outBaseConfig) const override;

        //////////////////////////////////////////////////////////////////////////
        // GradientRequestBus
        float GetValue(const GradientSampleParams& sampleParams) const override;
        void GetValues(AZStd::span<const AZ::Vector3> positions, AZStd::span<float> outValues) const override;

    protected:
        //////////////////////////////////////////////////////////////////////////
        // SurfaceTagGradientRequests
        size_t GetNumTags() const override;
        AZ::Crc32 GetTag(int tagIndex) const override;
        void RemoveTag(int tagIndex) override;
        void AddTag(AZStd::string tag) override;

    private:
        static float GetMaxSurfaceWeight(const SurfaceData::SurfacePointList& points)
        {
            float result = 0.0f;

            points.EnumeratePoints([&result](
                [[maybe_unused]] const AZ::Vector3& position, [[maybe_unused]] const AZ::Vector3& normal,
                const SurfaceData::SurfaceTagWeights& masks) -> bool
            {
                    masks.EnumerateWeights(
                        [&result]([[maybe_unused]] AZ::Crc32 surfaceType, float weight) -> bool
                        {
                            result = AZ::GetMax(AZ::GetClamp(weight, 0.0f, 1.0f), result);
                            return true;
                        });
                return true;
            });

            return result;
        }

        SurfaceMaskGradientConfig m_configuration;
        LmbrCentral::DependencyMonitor m_dependencyMonitor;
    };
}
