#pragma once

#include "types.hpp"

namespace realware
{
    namespace core
    {
        class cScene;

        class cLayer
        {

        public:
            cLayer() {}
            ~cLayer() {}

            virtual void Init(cScene* scene) = 0;
            virtual void Free(cScene* scene) = 0;
            virtual void Update(cScene* scene) = 0;

            void Toggle(boolean isEnabled) { m_IsEnabled = ~m_IsEnabled; }

            boolean GetState() { return m_IsEnabled; }

        private:
            boolean m_IsEnabled = K_TRUE;

        };
    }
}