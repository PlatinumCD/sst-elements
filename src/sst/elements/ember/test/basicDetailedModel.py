#!/usr/bin/env python
#
# Copyright 2009-2025 NTESS. Under the terms
# of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Copyright (c) 2009-2025, NTESS
# All rights reserved.
#
# Portions are copyright of other developers:
# See the file CONTRIBUTORS.TXT in the top level directory
# of the distribution for more information.
#
# This file is part of the SST software package. For license
# information, see the LICENSE file in the top level directory of the
# distribution.

import sst

from detailedModel import *

class BasicDetailedModel(DetailedModel):
    def __init__(self, params, nodeList ):
        self.name = 'BasicDetailedModel'
        self.params = params
        self.nodeList = nodeList
        self.links = []

    def getName(self):
        return self.name

    def createThreads(self, prefix, bus, numThreads, cpu_params, l1_params ):
        #print ("createThreads() ", prefix)
        prefix += "thread"
        links = []
        for i in range( numThreads ) :
            name = prefix + str(i) + "_"
            cpu = sst.Component( name + "cpu", "miranda.BaseCPU")
            cpu.addParams( cpu_params )
            l1 = sst.Component( name + "l1cache", "memHierarchy.Cache")
            l1.addParams(l1_params)

            link = sst.Link( name + "cpu_l1_link")
            link.setNoCut();
            link.connect( ( cpu, "cache_link", "100ps" ) , (l1,"highlink","1000ps") )

            link = sst.Link( name + "l1_bus_link")
            link.setNoCut();
            link.connect( ( l1, "lowlink", "50ps" ) , (bus,"highlink" + str(i+2),"1000ps") )

            link = sst.Link( name + "src_link" )
            link.setNoCut();
            cpu.addLink( link, "src", "1ps" );
            links.append( link )

        return links

    def createNic( self, prefix, op, bus, cpu_params, l1_params ):
        name = prefix + "nic_" + op + "_"
        #print ("createNic() ", name)

        cpu = sst.Component( name + "cpu", "miranda.BaseCPU")
        cpu.addParams( cpu_params )
        l1 = sst.Component( name + "l1cache", "memHierarchy.Cache")
        l1.addParams( l1_params )

        link = sst.Link( name + "cpu_l1_link")
        link.setNoCut();
        link.connect( ( cpu, "cache_link", "100ps" ) , (l1,"highlink","1000ps") )

        link = sst.Link( name + "l1_bus_link")
        link.setNoCut();
        if op == 'read':
            link.connect( ( l1, "lowlink", "50ps" ) , (bus,"highlink0","1000ps") )

        if op == 'write':
            link.connect( ( l1, "lowlink", "50ps" ) , (bus,"highlink1","1000ps") )

        link = sst.Link( name + "src_link" )
        link.setNoCut();
        cpu.addLink( link, "src", "1ps" );

        return link

    def build(self,nodeID,numCores):
        #print ('BasicDetailedModel.build( nodeID={0}, numCores={1} )'.format( nodeID, numCores ))

        self.links = []

        if not nodeID in self.nodeList:
            return False

        #print ("found", nodeID)

        prefix = "basicModel_node" + str(nodeID) + "_"

        memory = sst.Component( prefix + "memory", "memHierarchy.MemController")
        memory.addParams( self.params['memory_params'])

        membk = memory.setSubComponent("backend", "memHierarchy.simpleMem")
        membk.addParams( self.params['memory_backend_params'])

        l2 = sst.Component( prefix + "l2cache", "memHierarchy.Cache")
        l2.addParams( self.params['l2_params'])

        bus = sst.Component( prefix + "bus", "memHierarchy.Bus")
        bus.addParams( self.params['bus_params'])

        link = sst.Link( prefix + "bus_l2_link")
        link.setNoCut();
        link.connect( (bus, "lowlink0", "50ps"), (l2, "highlink", "50ps") )

        link = sst.Link( prefix + "l2_mem_link")
        link.setNoCut();
        link.connect( (l2, "lowlink", "50ps"), (memory, "highlink", "50ps") )

        for i in range(numCores):
            name = prefix + "core" + str(i) + "_"
            self.links.append( \
                self.createThreads( name, bus, int(self.params['numThreads']), \
                                    self.params['cpu_params'], \
                                    self.params['l1_params']  ) )

        self.nicReadLink = self.createNic( prefix, 'read', bus, self.params['nic_cpu_params'],\
                                    self.params['nic_l1_params'] )
        self.nicWriteLink = self.createNic( prefix, 'write', bus, self.params['nic_cpu_params'],\
                                    self.params['nic_l1_params'] )

        return True

    def getThreadLinks(self,core):
        #print ('BasicDetailedModel.getThreadLinks( {0} )'.format(core))
        return self.links[core]

    def getNicReadLink(self ):
        #print ('BasicDetailedModel.getNicLink()')
        return self.nicReadLink

    def getNicWriteLink(self ):
        #print ('BasicDetailedModel.getNicLink()')
        return self.nicWriteLink

def getModel(params,nodeList):
    return BasicDetailedModel(params,nodeList)
