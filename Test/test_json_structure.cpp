/*
 * test_json_structure.cpp
 *
 *  Created on: Mar 15, 2024
 *      Author: Nuno Barros
 */

#include <fstream>
#include <cstdio>

#include <json.hpp>
using json = nlohmann::json;

int main(int argc, char**argv)
{
  std::ifstream f("example.json");
  json conf = json::parse(f);

  std::string jsdata = conf.dump();
  printf("DUMP [%s]\n",jsdata.c_str());

  for (json::iterator it = conf.begin(); it != conf.end(); ++it)
  {
    if (it.key() == "server_address")
    {
      std::string host =  it.value();
      printf("host : %s\n",host.c_str());
    }
    if (it.key() == "id")
    {
      std::string id =  it.value();
      printf("id : %s\n",id.c_str());
    }
    if (it.key() == "coordinate_index")
    {
      uint16_t val=  it.value();
      printf("idx : %hu\n",val);
    }
    if (it.key() == "port")
    {
      uint16_t val=  it.value();
      printf("port : %hu\n",val);
    }
    if (it.key() == "range")
    {
      std::vector<int32_t> val=  it.value();
      std::vector<int32_t> val2=  it.value().get<std::vector<int32_t> >();
      printf("val size %lu\n",val.size());
      printf("range : %i %i\n",val[0],val[1]);

      printf("val2 size %lu\n",val2.size());
      for (auto i: val2)
      {
        printf("--> : %i\n",i);
      }
    }
    if (it.key() == "mmap")
    {
      // each entry has always 3 values
      // register, bit high, bit low
      json frag = it.value();
      std::string t= frag.dump();
      printf("DUMP [%s]\n",t.c_str());

      for (auto jt = frag.begin(); jt != frag .end(); ++jt)
      {
        printf("-------- %s -------\n",jt.key().c_str());
        std::vector<uint16_t> val=  jt.value();
        printf("val size %lu\n",val.size());
        for (auto i: val)
        {
          printf("--> : %hu\n",i);
        }
        printf("-------------------\n");
      }
    }
  }

  // test also dumping a map

  return 0;
}
