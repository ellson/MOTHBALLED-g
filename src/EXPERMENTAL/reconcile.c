#
#  connects edges between distribted systems
#


# start:
#    - one host asserts an edge-ACT that contains one or more ENDPOINTs
#      on remote systems

# reconciliate take place on a host that can (should be able to) reach
# all end systems.   This could be the initiating host,
# or the terminating host(s) (circumstances)
# or a third-party host, such as a viewer or query engine

# the results of reconciliation should be kept on the
# hosts that are members of the edge-ACT,  so that the results
# don't have to be recomputed by other views or query engines.

# (Does this me that edge-state is duplicated?)
# {What about the compund state?)




