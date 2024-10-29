#include "Open62541Client.h"

int main()
{
    try
    {
        Open62541Client client;
        client.connect("opc.tcp://localhost:4840");

        UA_Variant value;
        UA_Variant_init(&value);
        client.read_variable("the.node.id", value);
        // Process the value
        UA_Variant_clear(&value);

        client.disconnect();
    }
    catch (const std::exception &e)
    {
        spdlog::critical("Exception caught: {}", e.what());
    }

    return 0;
}
