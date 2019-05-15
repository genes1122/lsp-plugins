/*
 * CtlSource3D.h
 *
 *  Created on: 14 мая 2019 г.
 *      Author: sadko
 */

#ifndef UI_CTL_CTLSOURCE3D_H_
#define UI_CTL_CTLSOURCE3D_H_

namespace lsp
{
    namespace ctl
    {

        class CtlSource3D: public CtlWidget
        {
            protected:
                CtlColor        sColor;
                room_source_settings_t sSource;

                bool            bRebuildMesh;
                float           fPosX;
                float           fPosY;
                float           fPosZ;
                float           fYaw;
                float           fPitch;
                float           fRoll;

                CtlPort        *pMode;
                CtlPort        *pPosX;
                CtlPort        *pPosY;
                CtlPort        *pPosZ;
                CtlPort        *pYaw;
                CtlPort        *pPitch;
                CtlPort        *pRoll;
                CtlPort        *pCurvature;
                CtlPort        *pHeight;
                CtlPort        *pAngle;

            protected:
                void        update_source_location();
                void        update_mesh_data();

                static status_t slot_on_draw3d(LSPWidget *sender, void *ptr, void *data);

            public:
                explicit CtlSource3D(CtlRegistry *src, LSPMesh3D *widget);
                virtual ~CtlSource3D();

            public:
                virtual void init();

                virtual void set(widget_attribute_t att, const char *value);

                virtual void notify(CtlPort *port);
        };
    
    } /* namespace ctl */
} /* namespace lsp */

#endif /* UI_CTL_CTLSOURCE3D_H_ */