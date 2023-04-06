/*
 * cib_sc_client.cpp
 *
 *  Created on: Apr 5, 2023
 *      Author: nbarros
 */

// INFO
#define UA_LOGLEVEL 300

extern "C" {
#include <open62541.h>
};
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <cstring>

#include <json.hpp>

using json=nhlohmann::json;

/**
 * Auxiliary methods:
 *
 * list_endpoints : Searches for all endpoint objects that exist in the specified server
 * @return
 */
// This method iterates over the nodes
static UA_StatusCode
nodeIter(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle)
{
    if(isInverse)
        return UA_STATUSCODE_GOOD;
    UA_NodeId *parent = (UA_NodeId *)handle;
    printf("%u, %u --- %u ---> NodeId %u, %u\n",
           parent->namespaceIndex, parent->identifier.numeric,
           referenceTypeId.identifier.numeric, childId.namespaceIndex,
           childId.identifier.numeric);
    return UA_STATUSCODE_GOOD;
}


UA_StatusCode list_endpoints( UA_Client *client)
{
    /* Listing endpoints */
    /* This is a kind of discovery model */
    UA_EndpointDescription *endpointArray = NULL;
    size_t endpointArraySize = 0;
    UA_StatusCode retval = UA_Client_getEndpoints(client, "opc.tcp://localhost:4841",
                                                  &endpointArraySize, &endpointArray);
    if (retval != UA_STATUSCODE_GOOD)
    {
        UA_Array_delete(endpointArray, endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
        UA_Client_delete(client);
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    printf("%i endpoints found\n", (int)endpointArraySize);
    for (size_t i = 0; i < endpointArraySize; i++)
    {
        printf("URL of endpoint %i is %.*s\n", (int)i,
               (int)endpointArray[i].endpointUrl.length,
               endpointArray[i].endpointUrl.data);
    }
    UA_Array_delete(endpointArray, endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
    return retval;
}

UA_StatusCode browse_obj_loop(UA_Client *client)
{
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    /* Browse some objects */
    printf("Browsing nodes in objects folder:\n");
    UA_BrowseRequest bReq;
    UA_BrowseRequest_init(&bReq);
    bReq.requestedMaxReferencesPerNode = 0;
    bReq.nodesToBrowse = UA_BrowseDescription_new();
    bReq.nodesToBrowseSize = 1;
    bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); /* browse objects folder */
    bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL;                  /* return everything */
    UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);
    printf("%-9s %-16s %-16s %-16s\n", "NAMESPACE", "NODEID", "BROWSE NAME", "DISPLAY NAME");
    for (size_t i = 0; i < bResp.resultsSize; ++i)
    {
        for (size_t j = 0; j < bResp.results[i].referencesSize; ++j)
        {
            UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
            if (ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC)
            {
                printf("%-9d %-16d %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
                       ref->nodeId.nodeId.identifier.numeric, (int)ref->browseName.name.length,
                       ref->browseName.name.data, (int)ref->displayName.text.length,
                       ref->displayName.text.data);
            }
            else if (ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING)
            {
                printf("%-9d %-16.*s %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
                       (int)ref->nodeId.nodeId.identifier.string.length,
                       ref->nodeId.nodeId.identifier.string.data,
                       (int)ref->browseName.name.length, ref->browseName.name.data,
                       (int)ref->displayName.text.length, ref->displayName.text.data);
            }
            /* TODO: distinguish further types */
        }
    }
    UA_BrowseRequest_clear(&bReq);
    UA_BrowseResponse_clear(&bResp);
    return retval;
}

UA_StatusCode browse_obj_iterator(UA_Client *client)
{
    UA_StatusCode retval = UA_STATUSCODE_GOOD;

    /* Same thing, this time using the node iterator... */
    UA_NodeId *parent = UA_NodeId_new();
    *parent = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    retval |= UA_Client_forEachChildNodeCall(client, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                                   nodeIter, (void *)parent);
    UA_NodeId_delete(parent);

    return retval;
}

int main()
{
    UA_StatusCode retval = UA_STATUSCODE_GOOD;


    // a new client. that's us
    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));


    printf("Listing local server endpoints\n");
    retval = list_endpoints(client);
    if (retval != UA_STATUSCODE_GOOD) {
        printf("Failed\n");
        return EXIT_FAILURE;
    }

    /* Connect to a server */
    /* anonymous connect would be: retval = UA_Client_connect(client, "opc.tcp://localhost:4840"); */
    // retval = UA_Client_connectUsername(client, "opc.tcp://localhost:4840", "user1", "password");
    printf("Connecting to client\n");
    retval = UA_Client_connect(client, "opc.tcp://localhost:4841");
    if(retval != UA_STATUSCODE_GOOD) {
        printf("Failed\n");
        UA_Client_delete(client);
        return EXIT_FAILURE;
    }

    printf("Browsing objects with a loop\n");
    retval = browse_obj_loop(client);
    if (retval != UA_STATUSCODE_GOOD)
    {
        printf("Failed\n");
        UA_Client_delete(client);
        return EXIT_FAILURE;
    }

    printf("Browsing objects with an iterator\n");
    retval = browse_obj_iterator(client);
    if (retval != UA_STATUSCODE_GOOD)
    {
        printf("Failed\n");
        UA_Client_delete(client);
        return EXIT_FAILURE;
    }




    /* Read a node value */
    //
    // IMPORTANT: The node ids are set on the server side, and therefore need to be kept in sync
    //  		  for now most of the names are placeholders, so eventually we will need to have a meeting
    // 			  about cristalizing these


    /**
     * this is the typical way to read a value from the server
     * more complex values (arrays, strings) are slighly more involved, but the overall steps are these
     *
     * 1. Declare a variable of the appropriate type
     * 2. Declare a variant object (generic variable for internal use of the server
     * 3. Read the value
     * 4. Make basic checks that the value is good and valid
     * 5. Cast it into the variable
     */
    //
    //
    UA_Double value = 0.0;
    printf("\nReading the value of node for m1.speed:\n");
    UA_Variant *val = UA_Variant_new();
    retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(2, "L1.m1.speed"), val);

    if(retval == UA_STATUSCODE_GOOD && UA_Variant_isScalar(val) &&
       val->type == &UA_TYPES[UA_TYPES_DOUBLE]) {
            value = *(UA_Double*)val->data;
            printf("* the value is: %f\n", value);
    }
    UA_Variant_delete(val);




    /* Write node attribute */
    /**
     * This is is the generic, long windy way to do it
     * The advantage is that this way one can update multiple nodes/variables at once
     * I would still avoid it, if I could
     */
    // as example we will set the motor status refresh rate to 2000 ms
    UA_UInt16 motor_period_ms = 2000;
    printf("\nWriting a value to node for L1.m1.refresh_period_ms (setting to 2000):\n");
    UA_WriteRequest wReq;
    UA_WriteRequest_init(&wReq);
    wReq.nodesToWrite = UA_WriteValue_new();
    wReq.nodesToWriteSize = 1;
    wReq.nodesToWrite[0].nodeId = UA_NODEID_STRING(2, "L1.m1.refresh_period_ms");
    wReq.nodesToWrite[0].attributeId = UA_ATTRIBUTEID_VALUE;
    wReq.nodesToWrite[0].value.hasValue = true;
    wReq.nodesToWrite[0].value.value.type = &UA_TYPES[UA_TYPES_UINT16];
    wReq.nodesToWrite[0].value.value.storageType = UA_VARIANT_DATA_NODELETE; /* do not free the double on deletion */
    wReq.nodesToWrite[0].value.value.data = &motor_period_ms;
    // and now perform the write request
    UA_WriteResponse wResp = UA_Client_Service_write(client, wReq);

    if(wResp.responseHeader.serviceResult == UA_STATUSCODE_GOOD)
            printf("* the new value is: %d\n", motor_period_ms);
    UA_WriteResponse_clear(&wResp);



    // sleep for 5s
    std::cout << "----> Sleeping for 5s so one can see the previous refresh rate working" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));


    /* Write node attribute (using the highlevel API) */

    /**
     * this is the favored method to update a single value
     * Actually, I would recommend to do things this way in general
     *
     * The process is quite similar to reading:
     *
     * 1. Declate a variable with the value to be written
     * 2. Allocate a variant that will internally hold these values
     * 3. Copy the contents of the variable into the variant (one could use zero copy but can get tricky with async calls)
     * 4. Make a writing call to the server
     * 5. Clear the variant
     */
    printf("\nWriting another value to node for L1.m1.refresh_period_ms (setting to 5000 ms):\n");
    motor_period_ms = 5000;
    UA_Variant *myVariant = UA_Variant_new();
    UA_Variant_setScalarCopy(myVariant, &motor_period_ms, &UA_TYPES[UA_TYPES_UINT16]);
    // for an array the call would be
    // UA_StatusCode
    // UA_Variant_setArrayCopy(UA_Variant *v, const void * UA_RESTRICT array,
    //                        size_t arraySize, const UA_DataType *type)

    UA_Client_writeValueAttribute(client, UA_NODEID_STRING(2, "L1.m1.refresh_period_ms"), myVariant);
    UA_Variant_delete(myVariant);


    std::cout << "----> Sleeping for another 5s so one can see the previous refresh rate working" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));


    /// Read it back again
    printf("\nReading the value of node for L1.m1.refresh_period_ms:\n");
    UA_UInt16 v;
    val = UA_Variant_new();
    retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(2, "L1.m1.refresh_period_ms"), val);
    if (retval == UA_STATUSCODE_GOOD && UA_Variant_isScalar(val) &&

        val->type == &UA_TYPES[UA_TYPES_UINT16])
    {
        v = *(UA_UInt16 *)val->data;
        printf("--> the value is: %d\n", v);
    }
    UA_Variant_delete(val);

    printf("\nWriting another value to node for L1.m1.refresh_period_ms (setting to 0 ms, disabling get_info):\n");
    motor_period_ms = 0;
    myVariant = UA_Variant_new();
    UA_Variant_setScalarCopy(myVariant, &motor_period_ms, &UA_TYPES[UA_TYPES_UINT16]);
    UA_Client_writeValueAttribute(client, UA_NODEID_STRING(2, "L1.m1.refresh_period_ms"), myVariant);
    UA_Variant_delete(myVariant);


    std::cout << "----> Sleeping for another 5s so one can see the previous refresh rate working" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));


    /// Read it back again
    printf("\nReading the final value of node for L1.m1.refresh_period_ms:\n");
    //UA_UInt16 v;
    val = UA_Variant_new();
    retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(2, "L1.m1.refresh_period_ms"), val);
    if (retval == UA_STATUSCODE_GOOD && UA_Variant_isScalar(val) &&

        val->type == &UA_TYPES[UA_TYPES_UINT16])
    {
        v = *(UA_UInt16 *)val->data;
        printf("--> the final value is: %d\n", v);
    }
    UA_Variant_delete(val);











    /* Call a remote method */



    // The example method is the validate_config in motor 1
    // checks that the motor is ready to operate

    printf("-- \n\n Calling a method (start_move : L1.m1.start_move).\n");

    UA_UInt32 state;

    UA_Variant input;
    UA_Variant_init(&input);
    // UA_String argString = UA_STRING("Hello Server");
    // UA_Variant_setScalarCopy(&input, &argString, &UA_TYPES[UA_TYPES_STRING]);
    size_t outputSize;
    UA_Variant *output;

    /* prototype of the client call
        UA_StatusCode UA_Client_call(UA_Client * client, const UA_NodeId objectId, const UA_NodeId methodId, size_t inputSize, const UA_Variant *input, size_t *outputSize, UA_Variant **output)
    */
    retval = UA_Client_call(client, UA_NODEID_STRING(2, "L1.m1"),
    		UA_NODEID_STRING(2, "L1.m1.start_move"), 0, &input, &outputSize, &output);
    if(retval == UA_STATUSCODE_GOOD)
    {
        printf("Method call was successful, and %lu returned values available.\n",
               (unsigned long)outputSize);

        std::string ret((char*)static_cast<UA_String*>(output[0].data)->data,(size_t)static_cast<UA_String*>(output[0].data)->length);

        //std::string((char *)str->data, str->length);
        // All methods in the CIB return a json string
        //UA_String ret = *((UA_String *)output[0].data);
        //the string is non-null terminated in the UA_String object, so we need to do a mem copy
        //std::string sret(ret.data,)
        //char *ret_msg = new char[ret.length+1]; // need to allocate one more to add the null termination
        // seems silly but haven't found an alternative yet
        //std::memcpy(ret_msg,ret.data,ret.length);
        //ret_msg[ret.length] = '\0';
        printf(" --> Returned value %s\n", ret.c_str());
        //delete [] ret_msg;
        //printf(" --> Returned value %u\n", ret.*((UA_String *)output[0].data));
        //state = *((UA_UInt32 *)output[0].data);
        UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
    } else {
        printf("Method call was unsuccessful, and 0x%x was returned.\n", retval);
        printf("Message : %s\n", UA_StatusCode_name(retval));
    }
    UA_Variant_clear(&input);
//
//    // If the validation was successful, then set a target step and start moving the motor
//    if (state != 0x0)
//    {
//        printf("Configuration validation failed\n\n");
//    } else {
//        printf("Configuration validation passed. Initiating movement.\n\n");
//    }
//
//    // Set the target step to something ridiculously far
//    UA_NodeId nid = UA_NODEID_NUMERIC(2, 15294); // node id of the target_step variable
//    UA_UInt32 destination = 1544446;
//    val = UA_Variant_new();
//
//    // Actually, we could kind of automatize this a bit more, because we can query the
//    // server about the data type of this particular node
//    // But for now lets try this way
//
//    printf("--> Setting target step to %u.\n", destination);
//    UA_Variant_setScalarCopy(val, &destination, &UA_TYPES[UA_TYPES_UINT32]);
//    UA_Client_writeValueAttribute(client, nid, val);
//    UA_Variant_delete(val);
//
//    /// Read it back again to amke sure it was properly written
//    printf("\nReading the target step in motor 1:\n");
//    UA_UInt32 tstep;
//    val = UA_Variant_new();
//    retval = UA_Client_readValueAttribute(client,nid, val);
//    if (retval == UA_STATUSCODE_GOOD && UA_Variant_isScalar(val) &&
//        val->type == &UA_TYPES[UA_TYPES_INT32])
//    {
//        tstep = *((UA_UInt32 *)val->data);
//        printf("--> the value is: %u\n", tstep);
//    } else
//    {
//        printf("Something failed: ret %u scalar %u type %s\n", retval, UA_Variant_isScalar(val),(val->type)->typeName);
//    }
//    UA_Variant_delete(val);
//
//    if (tstep != destination) {
//        printf("Something unexpected happened. Values should be the same (%u != %u)\n",tstep,destination);
//    }
//    // All good. Call the method to start moving the motor
//    UA_NodeId_clear(&nid);
//
//    UA_NodeId obj_nid = UA_NODEID_NUMERIC(2, 15272); // node id of motor 1
//    UA_NodeId meth_id = UA_NODEID_NUMERIC(2, 15303); // node id for method start movement
//    UA_Variant_init(&input);
//    retval = UA_Client_call(client, obj_nid,
//                            meth_id, 0, &input, &outputSize, &output);
//    if (retval == UA_STATUSCODE_GOOD)
//    {
//        // there should be no return
//        printf("Method call was successful, and %lu returned values available.\n",
//               (unsigned long)outputSize);
//
//        UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
//
//        // -- if this was correct, now check that the node 'is_moving' (2,15300) is set to true
//        UA_Boolean is_moving = false;
//        nid = UA_NODEID_NUMERIC(2, 15300);
//        val = UA_Variant_new();
//        retval = UA_Client_readValueAttribute(client, nid, val);
//        if (retval == UA_STATUSCODE_GOOD && UA_Variant_isScalar(val) &&
//            val->type == &UA_TYPES[UA_TYPES_BOOLEAN])
//        {
//            is_moving = *((UA_Boolean *)val->data);
//        }
//        if (is_moving)
//        {
//            printf("\n\nMotor is moving\n\n");
//        } else {
//            printf("\n\nUnexpected. Motor is NOT moving\n\n");
//        }
//        UA_Variant_delete(val);
//        UA_NodeId_clear(&nid);
//    }
//    else
//    {
//        printf("Method call was unsuccessful, and 0x%x was returned.\n", retval);
//        printf("Message : %s\n", UA_StatusCode_name(retval));
//    }
//    UA_Variant_clear(&input);
//    UA_NodeId_clear(&obj_nid);
//    UA_NodeId_clear(&meth_id);

    printf("\n\n\n");

    printf("Let's try a complete sequence of operation.\n\n"
    		"2.Load a configuration and send it to the server\n"
    		"3.Check status\n"
    		"4.Set a target position\n"
    		"5.Call move_motor to set the movement\n"
    		"6.Periodically check the value of position to see what the motor is doing\n"
    );

    std::ifstream fconf("m1_config.json");
    json jconf =



    UA_Client_disconnect(client);
    UA_Client_delete(client);
    return EXIT_SUCCESS;

}
