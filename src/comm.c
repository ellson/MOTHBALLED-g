/* vim:set shiftwidth=4 ts=8 expandtab: */

//   Some rough ideas on comm model

//
//
//  g_deamon:              ^/session[]
//                          /stats[]
//                          /graph{ a b c <a b> }
//                          /net{
//                              user1@host1[
//                                  ipaddr=123.456.789.012
//                                  insocket=1024
//                                  outsocket=1025
//                                  type=ssh
//                                  key=ellson@work
//                                  created_by=user@host
//                                  ]
//                              user2@host2
//                              user3@host3
//                              user4@host4
//                              <user1@host1 user2@host2>
//                              ...
//                              }

// channel_types:  ssh, ssl, https
// port_type:  server, client
// poll_interval...
// refresh_interval... 


// short paths from graph:
//          ^ => ^/net
//          B => B/graph
//
// so:      ^/B/b   =>  ~/net/B/graph/b


// no way from graph to refer to ~/stats  or ~/session   ??? is this reasonable?
// 
//   how does anyone read or write to net/  ?
//   what do contained nets look like?
// 
//
// Fram another host's net/ :
//          B/graph/b
//          B/session   



///////////////////////// rethink .. may be inconsistent with above

// parser makes no assumption about existence or consistency of edges to nodes outside of the current host.
// To be dealt with by a (any) external proces that does have visibility across all hosts refenced by an edge.
// (An "annealer")


// parser writes "COUSIN" refs to local "ref" container....   makes no attempt at 
// communication.


// communication, status updates, edge reconciliation,  is the resposibility of another process:
// a  "viewer" or "annealer" 

// An annealer has write access to all nodes in the edge to be reconciled.
// An annealer can reconcile a distributed edge
//             - induce nodes in referenced hosts
//             - add reverse edge reference in referenced host
//                        - failure causes "broken" status to be added to originator
//                        - success updates all ends with "success" status, with timestamp in local times
// Annealing should be repeated at intervals
//             - default interval
//             - interval controlled by edge property

// Transfer of state
//      - state is carried by reconciled edge
//      - does the reconciliation process transfer state between connected nodes?
//          - or maybe, edge provides a state-transfer method that is triggered by the annealing process?


// state transfer method, stored in leg's external reference for edge: 
// leg = func(leg[*])  (where leg[*] refers to the connected nodes)

// The legs in each host can have their own functions.

