/*
 * DOMParser.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef DOMPARSER_H_
#define DOMPARSER_H_

#include "config.h"

#include "Node.h"
#include <libxml/xmlreader.h>
#include "../helpers/Path.h"

#include <vector>

namespace dash
{
    namespace xml
    {
        enum NodeType
        {
            Start           = 1,
            End             = 15,
            Comment         = 8,
            WhiteSpace      = 14,
            Text            = 3,
        };

        class DOMParser
        {
            public:
                DOMParser           (std::string url);
                DOMParser           (std::string url, const std::vector<unsigned char>& xmlFile);
                virtual ~DOMParser  ();

                bool    Parse                 ();
                bool    ParseFromMemory       ();
                Node*   GetRootNode           () const;
                void    Print                 ();

            private:
                xmlTextReaderPtr           reader;
                Node                       *root;
                std::string                url;
                std::vector<unsigned char> xmlFile;

                void    Init                    ();
                Node*   ProcessNode             ();
                void    AddAttributesToNode     (Node *node);
                void    Print                   (Node *node, int offset);
        };
    }
}
#endif /* DOMPARSER_H_ */
