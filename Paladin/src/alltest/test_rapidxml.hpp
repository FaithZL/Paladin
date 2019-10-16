#include <iostream>
#include <string>

#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "rapidxml_print.hpp"
  
using namespace std;

int test_rapidxml_main()
{
    using namespace rapidxml;
    xml_document<> doc1;
    xml_node<> *node = doc1.allocate_node(node_element,"a","Google Company");
    doc1.append_node(node);
  
    xml_attribute<> *attr = doc1.allocate_attribute("href","google.com");
    node->append_attribute(attr);
  
    //直接输出  
    cout<<"print:doc1 --> "<<doc1<<endl;
    
    //保存到磁盘
    ofstream out("res/xml_test_output.xml");//ofstream 默认时，如果文件存在则会覆盖原来的内容，不存在则会新建
    out << doc1;//doc 这样输出时在目标文件中不会有xml 头信息---<?xml version='1.0' encoding='utf-8' >
    out.close();

    //保存到string  
    string strxml;
    print(std::back_inserter(strxml),doc1,0);
    cout<<"print:strxml --> "<<strxml<<endl;

    file<> fdoc("res/xml_test_config.xml");
    xml_document<> doc2;
    doc2.parse<0>(fdoc.data());

    string text;
    rapidxml::print(std::back_inserter(text), doc2, 0);
    cout<<text<<std::endl;

    return 0;
}