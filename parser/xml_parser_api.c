#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>

void read_xml(char *attr, char *val)
{
    xmlDoc         *document;
    xmlNode        *root, *first_child, *node;
    char           *filename;
    xmlNode * child ;


    filename = "dummy.xml";

    document = xmlReadFile(filename, NULL, 0);
    root = xmlDocGetRootElement(document);
    fprintf(stdout, "Root is <%s> (%i)\n", root->name, root->type);
    first_child = root->children;
    for (node = first_child; node; node = node->next) {
        child = node->children;
        while(child)
        {
            //if(child->type == XML_ELEMENT_NODE) break;
            if (!strncmp(attr, child->name, strlen(child->name))) {
                strcpy(val, xmlNodeGetContent(child));
                break;
            }
               fprintf(stdout, "\tElem %s: %s\n", child->name, xmlNodeGetContent(child));
            child = child->next;
        }
        fprintf(stdout, "\t Child is <%s> (%i) content: %s\n", node->name, node->type, xmlNodeGetContent(node));
    }
    fprintf(stdout, "...\n");

}

