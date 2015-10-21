This document aims to present the general ideas and methodology that may
be used in order to design, develop and finally conduct performance
tests in order to optimize the metrics of each of the evaluation
criteria (i.e. reliability of the send data, end-to-end latency and and
power consumption).

[Wireless communication]

AUTHORS:
=======

Guillermo Aiello
-------------------------------------
Department of Network Engineering
Universitat Politecnica de Catalunya, Spain
gsie.etsetb@gmail.com

Ilker Demirkol
-------------------------------------
Department of Network Engineering
Universitat Politecnica de Catalunya, Spain
ilker.demirkol@entel.upc.edu

August Betzler
-------------------------------------
Mobile and Wireless Internet Unit
i2CAT Foundation, Spain
august.betzler@i2cat.net

Anna Calveras
-------------------------------------
Department of Network Engineering
Universitat Politecnica de Catalunya, Spain
anna.calveras@entel.upc.edu

Introduction 
============

According to EWSN 2016 Dependability challenge specifications, we devise
several solution candidates for the high-interference wireless sensor
network environments to achieve high energy-efficiency (measured as
average energy consumed per successful application layer packet
transmission) and low latency. The challenge uses off-the-shelf Maxfor
MTM-5000 sensor nodes, which bring some computational and memory
constraints.

Proposed Solutions 
==================

We take two approaches for this challenge: i) optimization of
standardized communication protocol stack through tweaking the
parameters of different protocols within stack as we have done in
@Betzler14, ii) a cross-layer approach that uses stateless routing as in
@Feng12. In the following we detail these two approaches. For the first
approach, we employ the protocols devised by Internet Engineering Task
Force (IETF) for Internet of Things (IoT) applications.

IETF IoT Protocol Stack
-----------------------

IETF has developed several solutions for each layer of the OSI protocol
stack considering the limitations of the IoT devices could bring. We
will be using the stack that is composed of
CoAP/UDP/IPv6-RPL/6LoWPAN/IEEE 802.15.4. As an option, the CoCoA
algorithm @Betzler15 proposed for congestion control over CoAP is also
considered within this stack. The derivation of optimal parameter
settings for different layers will be studied for
interference-environments. As an example, the RPL protocol parameters
investigated are minimum Trickle interval size (Imin), the RPL DIO
redundancy (K), and the Parent Switch Threshold (PST) defined by the
Objective Function used. These parameters determine how frequently the
RPL control messages are transmitted. The clear trade-off of network
stability vs. amount of overhead traffic is assessed within this work.

A clear solution for the MAC and PHY layers for interference environment
is the Timeslotted Channel Hopping (TSCH) mode of IEEE802.15.4e that has
been standardized in 2012. TSCH enables low-power operation through time
synchronization and channel hopping for high reliability. As the
targeted platform does not include an RF chip with this functionality,
one option considered in this work is the implementation of the MAC
layer of this standard.

Cross-layer Approach with Stateless Routing
-------------------------------------------

Even though using the inherit capabilities of the well-known IEEE
802.15.4-based stacks such as ZigBee or IETF IoT stack, one can
implement a functional solution for the targeted environment, it is
clear that there are inefficiency incurred, such as the encapsulation of
network, transport and application protocol packets, missing
communication of state information between layers, etc. Taking this into
account, a potentially simpler and leaner solution for the specific
application of the challenge can be developed.

For this, we consider a stateless routing protocol that incorporates the
MAC layer functionalities. Regarding the kind of topology and the
challenge requirements, a stateless routing is expected to considerably
increase the end-to-end performance, while allowing an easier
implementation after a short network discovery phase at the beginning.
The purpose of this discovery phase is to assign rank numbers (number of
hops to reach the sink) to nodes. The basic idea of the method is then
the following. When a sender node has some data ready to send, it sends
an RTS with its rank number. The available relay nodes with a higher
(and possibly equal) rank will send a CTS back, then the data
transmission will be carried out. This approach does not require any
neighbor information, nor routing table information, allowing an
asynchronous system and it is impervious to any broken links or nodes.
MAC reliability is enabled through ACK packets.

Another important aspect that must be taken into consideration is how
the interference nodes can be avoided. A channel hoping and duty cycling
approach will be used for this purpose.

Evaluation Methodology
======================

In order to devise and evaluate the solutions involving different
network topologies and different strategies, there are several
evaluation methodologies that can be used.

Simulation
----------

An available tool for the two operating systems for the constrained
nodes, TinyOS and ContikiOS, is Cooja Simulator. This software allows
the user to establish the desired network topology, with different
number of devices and surrounding objects, obstacles or interferences.
It also allows collecting the performance information such as delays,
errors and energy consumption, which are the target to beat within the
challenge context.

However the ultimate results of a simulation only match to predictions
and estimations based on the possible behavior, actually treating it as
much as possible with real capabilities.

Testbed
-------

Another approach to perform tests and check possible programming issues
involves the use of a group of physical nodes in which each of them are
separately programmed, including at least one sensing and one sink node
and few other intermediate node in order to test the behaviour of the
developed algorithms. However, a larger size TelosB testbed is available
in our university, consisting on a large grid topology of 6x10 TelosB
boards, already used in evaluations of @Betzler14. This testbed can be
used for physical environment tests and validation of simulation
results.

Programing OS environment
-------------------------

Due to embedded system capabilities regarding the available boards, two
OS options become potentially useful within this context: TinyOS and
ContikiOS. A further more detailed analysis would be performed in order
to choose which one fits better with the project requirements.

Acknowledgments
===============

This work was supported in part by the Spanish “Government’s Ministerio
de Economia y Competitividad” through project TEC2012-3253,
RYC-2013-13029 and FEDER.
