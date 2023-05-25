/*
 * BaseAccessControl.h
 *
 *  Created on: May 25, 2023
 *      Author: nbarros
 */

#ifndef SERVER_INCLUDE_ACCESSCONTROL_H_
#define SERVER_INCLUDE_ACCESSCONTROL_H_

#include <open62541.h>
#include <BaseAccessControl.h>

class AccessControl : public BaseAccessControl
{
public:
  AccessControl ();
  virtual ~AccessControl ();

  // in fact we can think of any context we want for this
  typedef struct {
      UA_Boolean allowAnonymous;
      size_t usernamePasswordLoginSize;
      UA_UsernamePasswordLogin *usernamePasswordLogin;
  } AccessControlContext;

  // the structure of the methods below is defined by the
  static UA_StatusCode
  activateSession(UA_Server *server, UA_AccessControl *ac,
                          const UA_EndpointDescription *endpointDescription,
                          const UA_ByteString *secureChannelRemoteCertificate,
                          const UA_NodeId *sessionId,
                          const UA_ExtensionObject *userIdentityToken,
                          void **sessionContext);


  static UA_Byte
  getUserAccessLevel(UA_Server *server, UA_AccessControl *ac,
                             const UA_NodeId *sessionId, void *sessionContext,
                             const UA_NodeId *nodeId, void *nodeContext);

  static UA_Boolean
  allowAddNode(UA_Server *server, UA_AccessControl *ac,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_AddNodesItem *item);

  static UA_Boolean
  allowAddReference(UA_Server *server, UA_AccessControl *ac,
                    const UA_NodeId *sessionId, void *sessionContext,
                    const UA_AddReferencesItem *item);

  static UA_Boolean
  allowDeleteNode(UA_Server *server, UA_AccessControl *ac,
                  const UA_NodeId *sessionId, void *sessionContext,
                  const UA_DeleteNodesItem *item);

  static UA_Boolean
  allowDeleteReference(UA_Server *server, UA_AccessControl *ac,
                       const UA_NodeId *sessionId, void *sessionContext,
                       const UA_DeleteReferencesItem *item);

  static UA_Boolean
  getUserExecutableOnObject(UA_Server *server, UA_AccessControl *ac,
                                    const UA_NodeId *sessionId, void *sessionContext,
                                    const UA_NodeId *methodId, void *methodContext,
                                    const UA_NodeId *objectId, void *objectContext);

  static UA_Boolean
  allowBrowseNode(UA_Server *server, UA_AccessControl *ac,
                          const UA_NodeId *sessionId, void *sessionContext,
                          const UA_NodeId *nodeId, void *nodeContext);

  static UA_Boolean
  getUserExecutable(UA_Server *server, UA_AccessControl *ac,
                            const UA_NodeId *sessionId, void *sessionContext,
                            const UA_NodeId *methodId, void *methodContext);
  // constants that we need to define


  void link_callbacks(UA_Server *s);

  static const UA_String anonymous_policy;
  static const UA_String username_policy;

protected:

private:

  AccessControl (const AccessControl &other) = delete;
  AccessControl& operator= (const AccessControl &other) = delete;
  AccessControl (AccessControl &&other) = delete;
  AccessControl& operator= (AccessControl &&other) = delete;
};

#endif /* SERVER_INCLUDE_ACCESSCONTROL_H_ */
