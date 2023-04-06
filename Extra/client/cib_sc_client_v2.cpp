/*
 * cib_sc_client_v2.cpp
 *
 *  Created on: Apr 5, 2023
 *      Author: nbarros
 *
 *      This client is more practical in the sense that it implements a more realistic set of instructions, without the
 *      redundancy of the other example.
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
#include <fstream>
#include <json.hpp>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using json = nlohmann::json;

//
//// This method iterates over the nodes
//static UA_StatusCode
//nodeIter(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle)
//{
//    if(isInverse)
//        return UA_STATUSCODE_GOOD;
//    UA_NodeId *parent = (UA_NodeId *)handle;
//    printf("%u, %u --- %u ---> NodeId %u, %u\n",
//           parent->namespaceIndex, parent->identifier.numeric,
//           referenceTypeId.identifier.numeric, childId.namespaceIndex,
//           childId.identifier.numeric);
//    return UA_STATUSCODE_GOOD;
//}
//
//
//UA_StatusCode list_endpoints( UA_Client *client)
//{
//    /* Listing endpoints */
//    /* This is a kind of discovery model */
//    UA_EndpointDescription *endpointArray = NULL;
//    size_t endpointArraySize = 0;
//    UA_StatusCode retval = UA_Client_getEndpoints(client, "opc.tcp://localhost:4841",
//                                                  &endpointArraySize, &endpointArray);
//    if (retval != UA_STATUSCODE_GOOD)
//    {
//        UA_Array_delete(endpointArray, endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
//        UA_Client_delete(client);
//        return UA_STATUSCODE_BADINTERNALERROR;
//    }
//    printf("%i endpoints found\n", (int)endpointArraySize);
//    for (size_t i = 0; i < endpointArraySize; i++)
//    {
//        printf("URL of endpoint %i is %.*s\n", (int)i,
//               (int)endpointArray[i].endpointUrl.length,
//               endpointArray[i].endpointUrl.data);
//    }
//    UA_Array_delete(endpointArray, endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
//    return retval;
//}
//
//UA_StatusCode browse_obj_loop(UA_Client *client)
//{
//    UA_StatusCode retval = UA_STATUSCODE_GOOD;
//    /* Browse some objects */
//    printf("Browsing nodes in objects folder:\n");
//    UA_BrowseRequest bReq;
//    UA_BrowseRequest_init(&bReq);
//    bReq.requestedMaxReferencesPerNode = 0;
//    bReq.nodesToBrowse = UA_BrowseDescription_new();
//    bReq.nodesToBrowseSize = 1;
//    bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); /* browse objects folder */
//    bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL;                  /* return everything */
//    UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);
//    printf("%-9s %-16s %-16s %-16s\n", "NAMESPACE", "NODEID", "BROWSE NAME", "DISPLAY NAME");
//    for (size_t i = 0; i < bResp.resultsSize; ++i)
//    {
//        for (size_t j = 0; j < bResp.results[i].referencesSize; ++j)
//        {
//            UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
//            if (ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC)
//            {
//                printf("%-9d %-16d %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
//                       ref->nodeId.nodeId.identifier.numeric, (int)ref->browseName.name.length,
//                       ref->browseName.name.data, (int)ref->displayName.text.length,
//                       ref->displayName.text.data);
//            }
//            else if (ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING)
//            {
//                printf("%-9d %-16.*s %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
//                       (int)ref->nodeId.nodeId.identifier.string.length,
//                       ref->nodeId.nodeId.identifier.string.data,
//                       (int)ref->browseName.name.length, ref->browseName.name.data,
//                       (int)ref->displayName.text.length, ref->displayName.text.data);
//            }
//            /* TODO: distinguish further types */
//        }
//    }
//    UA_BrowseRequest_clear(&bReq);
//    UA_BrowseResponse_clear(&bResp);
//    return retval;
//}
//
//UA_StatusCode browse_obj_iterator(UA_Client *client)
//{
//    UA_StatusCode retval = UA_STATUSCODE_GOOD;
//
//    /* Same thing, this time using the node iterator... */
//    UA_NodeId *parent = UA_NodeId_new();
//    *parent = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
//    retval |= UA_Client_forEachChildNodeCall(client, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
//                                   nodeIter, (void *)parent);
//    UA_NodeId_delete(parent);
//
//    return retval;
//}

int main()
{

    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    string server = "opc.tcp://localhost:4841";

    // create a new client, and use the default configuration options
    // for now this is fine
    // a new client. that's us
    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));

    /* Connect to the server */
    /* anonymous connect would be: retval = UA_Client_connect(client, "opc.tcp://localhost:4840"); */
    // retval = UA_Client_connectUsername(client, "opc.tcp://localhost:4840", "user1", "password");

    cout << "Connecting to CIB server..." << endl;

    retval = UA_Client_connect(client, server.c_str());

    if(retval != UA_STATUSCODE_GOOD)
    {
        cout << "Failed with code " << retval
				<< " name : [" << UA_StatusCode_name(retval) << "]" << endl;
        // we can print a message why
        UA_Client_delete(client);
        return EXIT_FAILURE;
    } else
    {
    	cout << "Success"<< endl;
    }


    // TODO: Define a variable that describes the status of the motor (waiting config, idle, moving)
    // for now we are assuming that we know what we are doing...LOL


    /**
     * Now we're in business. Let's rock!
     *
     *
     */
    cout << "Let's try a complete sequence of operation.\n\n"
    		"2.Load a configuration and send it to the server\n"
    		"3.Check status\n"
    		"4.Set a target position\n"
    		"5.Call move_motor to set the movement\n"
    		"6.Periodically check the value of position to see what the motor is doing\n"
    << endl;

    std::ifstream fconf("m1_config.json");
    json jconf = json::parse(fconf);
    cout << "Passing configuration :\n" << jconf.dump() << endl;
    // call the configuration method on motor 1
    printf("-- \n\n Calling a method (configure_motor : L1.m1.configure_motor).\n");

    UA_UInt32 state;

    // pass our configuration argument
    UA_Variant input;
    UA_Variant_init(&input);

	UA_String argString = UA_String_fromChars(jconf.dump().c_str());
	UA_Variant_setScalarCopy(&input, &argString, &UA_TYPES[UA_TYPES_STRING]);
    size_t outputSize;
    UA_Variant *output;

    /* prototype of the client call
        UA_StatusCode UA_Client_call(UA_Client * client,
        							const UA_NodeId objectId,
        							const UA_NodeId methodId,
        							size_t inputSize,
        							const UA_Variant *input,
        							size_t *outputSize,
        							UA_Variant **output)
    */
    retval = UA_Client_call(client, UA_NODEID_STRING(2, "L1.m1"),
    		UA_NODEID_STRING(2, "L1.m1.configure_motor"), 1, &input, &outputSize, &output);
    if(retval == UA_STATUSCODE_GOOD)
    {
    	cout << " Method call was successful, and got " << outputSize << "  returned values (expect 1)" << endl;


        // All methods in the CIB return a json string
        std::string response((char*)static_cast<UA_String*>(output[0].data)->data,(size_t)static_cast<UA_String*>(output[0].data)->length);
        cout << "Received response : " << response <<endl;
        json jresp = json::parse(response);

        cout << "Command executed with status : " << jresp.at("status") << " and messages : " <<endl;
        for (auto e : jresp.at("messages"))
        {
        	cout << "--> [" << e << "]" << endl;
        }
        UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
    } else
    {
        cout << "Failed with code 0x" << std::hex << retval << std::dec
 				<< " name : [" << UA_StatusCode_name(retval) << "]" << endl;

    }
    UA_Variant_clear(&input);
    fconf.close();

    /**
     * Read the current position
     */
    /// Read it back again
     cout << "\nReading the value of node for L1.m1.position " << endl;
     UA_Int32 current_pos = -99999;
     UA_Variant *val = UA_Variant_new();
     retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(2, "L1.m1.position"), val);
     if (retval == UA_STATUSCODE_GOOD)
     {
     	if ( UA_Variant_isScalar(val))
     	{
     		if (val->type == &UA_TYPES[UA_TYPES_INT32])
     		{
     	        current_pos = *static_cast<UA_Int32*>(val->data);
     	        cout << "Received L1.m1.position = " << current_pos << endl;
     		}
     		else
     		{
     			cout << "ERROR : Received data type was reported as something other than Int32 "
     					<< (val->type)->typeName << endl;
     		}
     	}
     	else
     	{
 			cout << "ERROR : Received a non-scalar result (empty : " << UA_Variant_isEmpty(val) << ") " << endl;
 			// this is a reasonable result, if the motor is not accessible
     	}
     }
     else
     {
     	cout << "Something went wrong and it should be investigated but I am too lazy to do it." << endl;
         cout << "Failed with code 0x" << std::hex << retval << std::dec
   				<< " name : [" << UA_StatusCode_name(retval) << "]" << endl;

     }
     UA_Variant_delete(val);


    /**
     * Set the position target point
     *
     */
    cout << "\n\nSetting target destination to 10000" << endl;
    UA_Int32 target_pos = 10000;
    UA_Variant *myVariant = UA_Variant_new();
    UA_Variant_setScalarCopy(myVariant, &target_pos, &UA_TYPES[UA_TYPES_INT32]);

    // for an array the call would be
    // UA_StatusCode
    // UA_Variant_setArrayCopy(UA_Variant *v, const void * UA_RESTRICT array,
    //                        size_t arraySize, const UA_DataType *type)

    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(2, "L1.m1.positionSetPoint"), myVariant);
    if(retval == UA_STATUSCODE_GOOD)
    {
    	cout << " Target position set to  call was successful" << endl;
    } else
    {
        cout << "Failed with code 0x" << std::hex << retval << std::dec
 				<< " name : [" << UA_StatusCode_name(retval) << "]" << endl;

    }
    UA_Variant_delete(myVariant);


    std::this_thread::sleep_for(std::chrono::milliseconds(10));


    /// Read it back again
    cout << "\nReading the value of node for L1.m1.positionSetPoint to be sure it is what I set it to be" << endl;
    UA_Int32 psetp; // new variable to make sure I am not cheating
    val = UA_Variant_new();
    retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(2, "L1.m1.positionSetPoint"), val);
    if (retval == UA_STATUSCODE_GOOD)
    {
    	if ( UA_Variant_isScalar(val))
    	{
    		if (val->type == &UA_TYPES[UA_TYPES_INT32])
    		{
    	        psetp = *static_cast<UA_Int32*>(val->data);
    	        cout << "Received L1.m1.positionSetPoint = " <<  psetp << endl;

    		}
    		else
    		{
    			cout << "ERROR : Received data type was reported as something other than Int32 "
    					<< (val->type)->typeName << endl;
    		}
    	}
    	else
    	{
			cout << "ERROR : Received a non-scalar result (empty : " << UA_Variant_isEmpty(val) << ") " << endl;

    	}
    }
    else
    {
    	cout << "Something went wrong and it should be investigated but I am too lazy to do it." << endl;
        cout << "Failed with code 0x" << std::hex << retval << std::dec
  				<< " name : [" << UA_StatusCode_name(retval) << "]" << endl;

    }
    UA_Variant_delete(val);





    // now that we have configured and have set a destination, let's tell it to move

    // call the configuration method on motor 1
    printf("-- \n\n Calling a method (start_move : L1.m1.start_move).\n");

    // pass our configuration argument
    // reuse the variables of the first call
    UA_Variant_init(&input);

    retval = UA_Client_call(client, UA_NODEID_STRING(2, "L1.m1"),
    		UA_NODEID_STRING(2, "L1.m1.start_move"), 0, &input, &outputSize, &output);
    if(retval == UA_STATUSCODE_GOOD)
    {
    	cout << " Method call was successful, and got " << outputSize << "  returned values (expect 1)" << endl;

        // All methods in the CIB return a json string
        std::string response((char*)static_cast<UA_String*>(output[0].data)->data,(size_t)static_cast<UA_String*>(output[0].data)->length);
        cout << "Received response : " << response <<endl;
        json jresp = json::parse(response);

        cout << "Command executed with status : " << jresp.at("status") << " and messages : " <<endl;
        for (auto e : jresp.at("messages"))
        {
        	cout << "--> [" << e << "]" << endl;
        }
    } else
    {
        cout << "Failed with code 0x" << std::hex << retval << std::dec
 				<< " name : [" << UA_StatusCode_name(retval) << "]" << endl;
        // note that even if it failed, the return variant may still carry out a response
//        cout << "Method reports " << outputSize << " response objects." << endl;
//
//
//        // All methods in the CIB return a json string
//        std::string response((char*)static_cast<UA_String*>(output[0].data)->data,(size_t)static_cast<UA_String*>(output[0].data)->length);
//        cout << "Received response : " << response <<endl;
//        json jresp = json::parse(response);
//
//        cout << "Command executed with status : " << jresp.at("status") << " and messages : " <<endl;
//        for (auto e : jresp.at("messages"))
//        {
//        	cout << "--> [" << e << "]" << endl;
//        }
//        UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);


    }
    UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
    UA_Variant_clear(&input);





    printf("-- \n\n Periodically checking the current position : L1.m1.position).\n");


    // -- if this was going well now we could set a loop and check where is the position
    bool stop = false;
    while ((current_pos != target_pos) && !stop)
    {
    	// read back the position every 500 ms
    	// note that the CIB itself is only querying the server a set period, so there is a chance that we are asking
    	// more often than the CIB is getting updated. This would be visible in repeated reads of the same value

		val = UA_Variant_new();
		retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(2, "L1.m1.position"), val);
		if ((retval == UA_STATUSCODE_GOOD) && (val->type == &UA_TYPES[UA_TYPES_INT32]) && UA_Variant_isScalar(val))
		{
			current_pos = *static_cast<UA_Int32*>(val->data);
			cout << "Received L1.m1.position = " <<  current_pos << endl;
		}
		else
		{
			cout << "Failed with code 0x" << std::hex << retval << std::dec
    	   				<< " name : [" << UA_StatusCode_name(retval) << "]" << endl;

			stop = true;
		}
		UA_Variant_delete(val);
	    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    cout << "Work is done. Time to go to the beach..." << endl;


    printf("\n\n\n");




    UA_Client_disconnect(client);
    UA_Client_delete(client);
    return EXIT_SUCCESS;

}
