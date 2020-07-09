#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>


#include "orchestrator.h"
#include "orch_cli.h"
#include "orch_xml_api.h"

orch_status_t orch_parse_xml(char *file, const char *attr, char *val)
{
    xmlDoc         *document;
    xmlNode        *root, *first_child, *node, *child;

    if (!file || !attr || !val)
        return ORCH_STATUS_ERROR;

    document = xmlReadFile(file, NULL, 0);
    if (document == NULL) {
        printf("Could not parse the xml file %s\n", file);
        return ORCH_STATUS_FAIL;
    }
    root = xmlDocGetRootElement(document);
    if (root == NULL) {
        printf("Could not parse the root of xml file %s\n", file);
        xmlFreeDoc(document);
        return ORCH_STATUS_FAIL;
    }
    first_child = root->children;
    for (node = first_child; node; node = node->next) {
        child = node->children;
        while(child)
        {
            if (!strncmp(attr, child->name, strlen(child->name))) {
                strcpy(val, xmlNodeGetContent(child));
                break;
            }
            child = child->next;
        }
        DEBUG_PRINT(" Looking xml attribute %s in file %s\n", attr, file);
        DEBUG_PRINT("Child (%s) : Content: %s\n", node->name, xmlNodeGetContent(node));
    }
    xmlFreeDoc(document);
    return ORCH_STATUS_SUCCESS;
}



