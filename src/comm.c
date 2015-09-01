
//   Some rough ideas on comm model

//
//
//  g_deamon:      user@host/session[]
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
