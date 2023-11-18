import matplotlib.pyplot as plt
import random
import numpy as np

class Client:
    def __init__(self, name):
        self.name = name
        self.path = []

    def __len__(self):
        return len(self.path)

    def get_vector(self,idx):
        return self.path[idx]

    def put_vector(self, position):
        self.path.append(position)

    def plot_path(self, path): #WIP this is a placeholder until we have a working regression system
        #path is a list of 2d point (x,y)
        x = []
        y = []
        for point in path:
            x.append(point[0])
            y.append(point[1])
        #scatter plot, plot each point with different color
        colors = [plt.cm.viridis(i) for i in np.linspace(0, 1, len(x))]
        plt.scatter(x, y, c=colors)

        for i, txt in enumerate(path):
            plt.text(x[i], y[i], str(i), fontsize=12)
            
        plt.show()

