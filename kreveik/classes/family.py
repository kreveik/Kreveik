"""
Definition of the family class
"""
import numpy as num
import matplotlib.pyplot as plt
import logging
import classes
import probes


class Family(classes.ProbeableObj,classes.Ensemble):
    '''
    Family Class.
    A class that will have a single family of networks, a genetic algorithm 
    will be run within a family. The macro parameters of a group of network, 
    though may also be reached from a list of networks, will be housed in a 
    family in a more structured manner.
    '''
    
    def __init__ (self):
        classes.ProbeableObj.__init__(self)
        classes.Ensemble.__init__(self)
        self.network_list = []
        self.wildtype_list = [] 
        self.scores = num.array([])
        self.scores_history = []
        
            
    def __getitem__(self, index):
        """
        nth item of a Family object returns the nth network it has
        """
        if index > len(self.network_list):
            raise IndexError
        return self.network_list[index]
    
    def __contains__(self, network):
        """
        Returns a boolean according to whether a family includes a network  
        """
        return network in self.network_list
        
    
    def add(self, network):
        '''
        This method will add a network to the specified family.
        network -> The network that is to be appended to the family.
        '''
        
        if network in self.network_list:
            logging.warning("The network that you're trying to add to the family is already a member.\
            Please clone the individual and append the clone.")
            return False
        
        self.network_list.append(network)
        return True
            
    def plot_scores(self):
             
        plt.plot(self.scores)
        plt.show()
    
    def populate_equilibria(self):
        '''
        If the family has individuals, it goes to each individual and finds the equilibria 
        for all possible initial conditions they may face. The orbits and scores are 
        assigned to each one of them.
        '''
        
        if len(self.network_list) == 0:
            logging.warning("Warning: There is nobody in this family.Please adopt individuals.")
            return False
            
        self.scores = num.zeros(len(self.network_list))
        
        for counter,network in enumerate(self.network_list): 
            logging.info("("+str(counter+1)+"/"+str(len(self.network_list))
                         +") Populating equilibrium for: "+str(network))
            network.populate_equilibria()
            self.scores[counter] = network.score
        self.populate_probes(probes.populate_equilibria_in_family)
            


