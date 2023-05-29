/*
 * BaseAccessControl.cpp
 *
 *  Created on: May 25, 2023
 *      Author: nbarros
 */

#include <LogIt.h>
#include <statuscode.h>
#include <AccessControl.h>

const UA_String AccessControl::anonymous_policy = UA_STRING_STATIC("open62541-anonymous-policy");
const UA_String AccessControl::username_policy = UA_STRING_STATIC("open62541-username-policy");

AccessControl::AccessControl ()
{
  ////LOG(Log::INF) << "AccessControl::AccessControl : Building access control object.";
}

AccessControl::~AccessControl ()
{
  // TODO Auto-generated destructor stub
}


UA_StatusCode
AccessControl::activateSession(UA_Server *server, UA_AccessControl *ac,
                        const UA_EndpointDescription *endpointDescription,
                        const UA_ByteString *secureChannelRemoteCertificate,
                        const UA_NodeId *sessionId,
                        const UA_ExtensionObject *userIdentityToken,
                        void **sessionContext) {
  ////LOG(Log::ERR) << "AccessControl::activateSession : Activating session.";

  AccessControl::AccessControlContext *context = (AccessControl::AccessControlContext*)ac->context;

    /* The empty token is interpreted as anonymous */
    if(userIdentityToken->encoding == UA_EXTENSIONOBJECT_ENCODED_NOBODY)
    {
      ////LOG(Log::INF) << "activateSession : Received an empty token.";
        if(!context->allowAnonymous)
        {
          ////LOG(Log::ERR) << "activateSession : anonymous tokens are not accepted. Returning invalid.";
            return UA_STATUSCODE_BADIDENTITYTOKENINVALID;
        }
        /* No userdata atm */
        //TODO: Allow anonymous but add a context that restricts the usage
//        char* anonymous_string = "anonymous";
//        UA_ByteString* bstr = UA_ByteString_new();
//        bstr->data = (UA_Byte*) anonymous_string;
//        bstr->length = 10;
//
//        UA_ByteString *username = UA_ByteString_new();
        UA_ByteString username = UA_BYTESTRING_ALLOC("anonymous");
//        if(username)
//          UA_ByteString_copy("anonymous", username);
        //printf("Context will have [%s] [%s]\n\n",userToken->userName.data,username->data);
        *sessionContext = &username;

        //*sessionContext = NULL;
        ////LOG(Log::WRN) << "activateSession : Allowing anonymous token. sessionContext will be empty. Returning good.";

        return UA_STATUSCODE_GOOD;
    }

    /* Could the token be decoded? */
    if(userIdentityToken->encoding < UA_EXTENSIONOBJECT_DECODED)
    {
      ////LOG(Log::ERR) << "activateSession : Token could not be decoded. Returning invalid.";
      return UA_STATUSCODE_BADIDENTITYTOKENINVALID;
    }
    /* Anonymous login */
    if(userIdentityToken->content.decoded.type == &UA_TYPES[UA_TYPES_ANONYMOUSIDENTITYTOKEN])
    {
      ////LOG(Log::INF) << "activateSession : Received an anonymous token.";
        if(!context->allowAnonymous)
        {
          ////LOG(Log::ERR) << "activateSession : anonymous tokens are not accepted. Returning invalid.";
           return UA_STATUSCODE_BADIDENTITYTOKENINVALID;
        }
        const UA_AnonymousIdentityToken *token = (UA_AnonymousIdentityToken*)
            userIdentityToken->content.decoded.data;

        /* Compatibility notice: Siemens OPC Scout v10 provides an empty
         * policyId. This is not compliant. For compatibility, assume that empty
         * policyId == ANONYMOUS_POLICY */
        if(token->policyId.data && !UA_String_equal(&token->policyId, &AccessControl::anonymous_policy))
        {
          //LOG(Log::ERR) << "activateSession : Received an anonymous policy token. Returning invalid.";

            return UA_STATUSCODE_BADIDENTITYTOKENINVALID;
        }
        /* No userdata atm */
       // *sessionContext = NULL;
        UA_ByteString username = UA_BYTESTRING_ALLOC("anonymous");
        *sessionContext = &username;

        //LOG(Log::WRN) << "activateSession : Allowing anonymous policy token. sessionContext will be empty. Returning good.";

        return UA_STATUSCODE_GOOD;
    }

    /* Username and password */
    if(userIdentityToken->content.decoded.type == &UA_TYPES[UA_TYPES_USERNAMEIDENTITYTOKEN]) {
      //printf("\n\nTrying a username/password \n");
      //LOG(Log::INF) << "activateSession : Received an username/password token.";


        const UA_UserNameIdentityToken *userToken =
            (UA_UserNameIdentityToken*)userIdentityToken->content.decoded.data;
        //LOG(Log::INF) << "activateSession : Decoded data : " << userToken->userName.data
        //    << ":" << userToken->password.data;

        if(!UA_String_equal(&userToken->policyId, &AccessControl::username_policy))
        {
          //LOG(Log::ERR) << "activateSession : policyId does not match username policy. Returning invalid.";

            return UA_STATUSCODE_BADIDENTITYTOKENINVALID;
        }
        /* The userToken has been decrypted by the server before forwarding
         * it to the plugin. This information can be used here. */
        /* if(userToken->encryptionAlgorithm.length > 0) {} */

        /* Empty username and password */
        if(userToken->userName.length == 0 && userToken->password.length == 0)
        {
          //LOG(Log::ERR) << "activateSession : user/pwd token with 0 length : (" << userToken->userName.length << ":"<<  userToken->password.length << ")";

            return UA_STATUSCODE_BADIDENTITYTOKENINVALID;
        }
        /* Try to match username/pw */
        UA_Boolean match = false;
        for(size_t i = 0; i < context->usernamePasswordLoginSize; i++) {
            if(UA_String_equal(&userToken->userName, &context->usernamePasswordLogin[i].username) &&
               UA_String_equal(&userToken->password, &context->usernamePasswordLogin[i].password)) {
              //printf("\n\nFound a match\n");

              match = true;
                break;
            }
        }
        if(!match)
        {
          //LOG(Log::ERR) << "activateSession : Didn't find any user/pwd match.";

          return UA_STATUSCODE_BADUSERACCESSDENIED;

        }

        /* For the CTT, recognize whether two sessions are  */
        UA_ByteString *username = UA_ByteString_new();
        if(username)
            UA_ByteString_copy(&userToken->userName, username);
        //printf("Context will have [%s] [%s]\n\n",userToken->userName.data,username->data);
        *sessionContext = username;
        //printf("Context has [%s]\n\n",static_cast<UA_ByteString *>(*sessionContext)->data);
        //LOG(Log::INF) << "activateSession : all good. sessionContext is set.";

        return UA_STATUSCODE_GOOD;
    }

    /* Unsupported token type */
    //LOG(Log::ERR) << "activateSession : Unsupported token. Returning invalid.";

    return UA_STATUSCODE_BADIDENTITYTOKENINVALID;
}

UA_Byte
AccessControl::getUserAccessLevel(UA_Server *server, UA_AccessControl *ac,
                           const UA_NodeId *sessionId, void *sessionContext,
                           const UA_NodeId *nodeId, void *nodeContext) {

  UA_String strId;
  UA_String_init(&strId);
  UA_NodeId_print(sessionId, &strId);
  UA_String str_nodeId;
  UA_String_init(&str_nodeId);
  UA_NodeId_print(nodeId, &str_nodeId);

  //printf("Checking permission for session %s and context %p %s\n",strId.data,sessionContext, static_cast<UA_ByteString *>(sessionContext)->data);
  //printf("node %s and context %p \n",str_nodeId.data, nodeContext);

  UA_String *context = static_cast<UA_ByteString *>(sessionContext);
  UA_String  u = UA_String_fromChars("nuno");
  if (UA_String_equal(context,&u ))
  {
    return 0xFF; // permit everything
  }
  else
  {
    // everyone else can only read
    return UA_ACCESSLEVELMASK_READ;
  }
}

UA_Boolean
AccessControl::allowAddNode(UA_Server *server, UA_AccessControl *ac,
             const UA_NodeId *sessionId, void *sessionContext,
             const UA_AddNodesItem *item) {
    printf("Called allowAddNode\n");
    // nobody can add nodes
    return UA_FALSE; //UA_TRUE;
}

UA_Boolean
AccessControl::allowAddReference(UA_Server *server, UA_AccessControl *ac,
                  const UA_NodeId *sessionId, void *sessionContext,
                  const UA_AddReferencesItem *item) {
    printf("Called allowAddReference\n");
    return UA_FALSE; //UA_TRUE;
}

UA_Boolean
AccessControl::allowDeleteNode(UA_Server *server, UA_AccessControl *ac,
                const UA_NodeId *sessionId, void *sessionContext,
                const UA_DeleteNodesItem *item) {
    printf("Called allowDeleteNode\n");
    return UA_FALSE; // Do not allow deletion from client
}

UA_Boolean
AccessControl::allowDeleteReference(UA_Server *server, UA_AccessControl *ac,
                     const UA_NodeId *sessionId, void *sessionContext,
                     const UA_DeleteReferencesItem *item) {
    printf("Called allowDeleteReference\n");
    return UA_FALSE; //UA_TRUE;
}

UA_Boolean
AccessControl::getUserExecutableOnObject(UA_Server *server, UA_AccessControl *ac,
                                  const UA_NodeId *sessionId, void *sessionContext,
                                  const UA_NodeId *methodId, void *methodContext,
                                  const UA_NodeId *objectId, void *objectContext) {

  UA_String *context = static_cast<UA_ByteString *>(sessionContext);
  UA_String  u = UA_String_fromChars("nuno");
  bool is_operator= UA_String_equal(context,&u )?true:false;

   // UA_StatusCode UA_Server_readNodeClass(UA_Server *server, const UA_NodeId nodeId,
  //                        UA_NodeClass *outNodeClass)
  UA_QualifiedName nqname;
  UA_Server_readBrowseName(server,*methodId,&nqname);
  //  UA_Server_readBrowseName(UA_Server *server, const UA_NodeId nodeId,
  //                           UA_QualifiedName *outBrowseName)

//  printf("\n\nBrowse result for node %s and user %s : %d\n\n",nqname.name.data,context->data,is_operator);

  return is_operator;
//
//  UA_String str_nodeId;
//  UA_String_init(&str_nodeId);
//  UA_NodeId_print(methodId, &str_nodeId);
//
//  UA_String str_objId;
//  UA_String_init(&str_objId);
//  UA_NodeId_print(objectId, &str_objId);
//
//  printf("Checking permission for method %s obj %s context %p %s\n",str_nodeId.data,str_objId.data,sessionContext, static_cast<UA_ByteString *>(sessionContext)->data);
//  //printf("method %s and context %p \n",str_nodeId.data);
//  UA_String *context = static_cast<UA_ByteString *>(sessionContext);
//  UA_String  u = UA_String_fromChars("nuno");
//  if (UA_String_equal(context,&u ))
//  {
//    printf("\n\nDeclining execute permissions on object\n\n");
//    return true;
//  }
//  else {
//    return false;
//  }
}

UA_Boolean
AccessControl::allowBrowseNode(UA_Server *server, UA_AccessControl *ac,
                        const UA_NodeId *sessionId, void *sessionContext,
                        const UA_NodeId *nodeId, void *nodeContext) {

  // TODO: Find if there is a way of figuring out
  // which nodes are methods, to not even allow them to be browsed by
  UA_String str_nodeId;
  UA_String_init(&str_nodeId);
  UA_NodeId_print(nodeId, &str_nodeId);

//  printf("Checking browse permission for nodeId %s session context %p %s node context %p\n",
//         str_nodeId.data,
//         sessionContext,
//         static_cast<UA_ByteString *>(sessionContext)->data,
//         nodeContext);

  // -- try to figure out if node is a method
  //UA_ServerConfig* config = UA_Server_getConfig(server);
  // from the config we can get the nodeStore
  //const UA_Node * node = config->nodestore.getNode(config->nodestore.context,nodeId);

  // release the pointer
  //config->nodestore.releaseNode(config->nodestore.context,node);

  ///
  /// Method 2: Check directly the NodeClass
  ///
  UA_NodeClass nclass;
  UA_Server_readNodeClass(server,*nodeId,&nclass);

  bool is_method = (nclass == UA_NODECLASS_METHOD)?true:false;
  UA_String *context = static_cast<UA_ByteString *>(sessionContext);
  UA_String  u = UA_String_fromChars("nuno");
  bool is_operator= UA_String_equal(context,&u )?true:false;

  bool result = (!is_method || (is_method && is_operator));

  // UA_StatusCode UA_Server_readNodeClass(UA_Server *server, const UA_NodeId nodeId,
  //                        UA_NodeClass *outNodeClass)
  UA_QualifiedName nqname;
  UA_Server_readBrowseName(server,*nodeId,&nqname);
  //  UA_Server_readBrowseName(UA_Server *server, const UA_NodeId nodeId,
  //                           UA_QualifiedName *outBrowseName)

//  printf("\n\nBrowse result for node %s and user %s : %d\n\n",nqname.name.data,context->data,result);

  return result;

//  if (nclass == UA_NODECLASS_METHOD)
//  {
//    printf("\n\n\n Node %s is a method\n\n\n",nqname.name.data);
//    return false;
//  }
//  else
//  {
//    printf("\n\n\n Node %s is NOT a method\n\n\n",nqname.name.data);
//
//  }
////  UA_Server_readNodeClass(UA_Server *server, const UA_NodeId nodeId,
////                          UA_NodeClass *outNodeClass);
//  //UA_String_delete(&str_nodeId);
//  return true;
//
//
//
//  printf("Checking browse permission for nodeId %s session context %p %s node context %p\n",
//         str_nodeId.data,
//         sessionContext,
//         static_cast<UA_ByteString *>(sessionContext)->data,
//         nodeContext);
//  //printf("method %s and context %p \n",str_nodeId.data);
//  UA_String *context = static_cast<UA_ByteString *>(sessionContext);
//  UA_String  u = UA_String_fromChars("nuno");
//  if (UA_String_equal(context,&u ))
//  {
//    return true;
//  }
//  else {
//    printf("\n\nDeclining browse permissions\n\n");
//    return false;
//  }
}

UA_Boolean
AccessControl::getUserExecutable(UA_Server *server, UA_AccessControl *ac,
                          const UA_NodeId *sessionId, void *sessionContext,
                          const UA_NodeId *methodId, void *methodContext) {
//  printf("\n\nCalled getUserExecutable\n\n");


  UA_String str_nodeId;
  UA_String_init(&str_nodeId);
  UA_NodeId_print(methodId, &str_nodeId);

//  printf("Checking permission for method %s and context %p %s\n",str_nodeId.data,sessionContext, static_cast<UA_ByteString *>(sessionContext)->data);
  //printf("method %s and context %p \n",str_nodeId.data);
  UA_String *context = static_cast<UA_ByteString *>(sessionContext);
  UA_String  u = UA_String_fromChars("nuno");
  if (UA_String_equal(context,&u ))
  {
    return true;
  }
  else {
//    printf("\n\nDeclining execute permissions\n\n");
    return false;
  }
}

void AccessControl::link_callbacks(UA_Server *s)
{
  //LOG(Log::INF) <<  "AccessControl::link_callbacks : Linking callbacks";
  UA_ServerConfig* config = UA_Server_getConfig(s);

  UA_UsernamePasswordLogin logins[2] = {
      {UA_STRING_STATIC("peter"), UA_STRING_STATIC("peter123")},
      {UA_STRING_STATIC("nuno"), UA_STRING_STATIC("nuno123")}
  };


  // clear whatever exists in terms of access control
  config->accessControl.clear(&config->accessControl);
  UA_StatusCode retval = UA_AccessControl_default(config, false,
                 &config->securityPolicies[config->securityPoliciesSize-1].policyUri, 2, logins);
      if (retval != UA_STATUSCODE_GOOD)
      {
        LOG(Log::ERR) <<
        "UA_AccessControl_default returned: " << UaStatus(retval).toString().toUtf8();
      }
//      else
//      {
//        //LOG(Log::INF) <<
//        //"UA_AccessControl_default returned: " << UaStatus(retval).toString().toUtf8();
//
//      }

     // now attach the callbacks that matter

      config->accessControl.activateSession = AccessControl::activateSession;
      config->accessControl.getUserAccessLevel = AccessControl::getUserAccessLevel;
      config->accessControl.allowAddNode = AccessControl::allowAddNode;
      config->accessControl.allowAddReference = AccessControl::allowAddReference;
      config->accessControl.allowDeleteNode = AccessControl::allowDeleteNode;
      config->accessControl.allowDeleteReference = AccessControl::allowDeleteReference;
      config->accessControl.getUserExecutableOnObject = AccessControl::getUserExecutableOnObject;
      config->accessControl.allowBrowseNode = AccessControl::allowBrowseNode;
      config->accessControl.getUserExecutable = AccessControl::getUserExecutable;

  // initialize the
}
