import matplotlib.pyplot as plt
import random

class Client:
    def __init__(self, name):
        self.name = name
        self.path = []

    def get_vector(self,idx):
        return self.path[idx]

    def put_vector(self, position):
        self.path.append(position)

    def plot_path(self, path): #WIP this is a placeholder until we have a working regression system
        #path is a list of 2d point (x,y)
        x = []
        y = []
        for point in path:
            x.append(point[0]+random.uniform(-0.1,0.1))
            y.append(point[1]+random.uniform(-0.1,0.1))

        plt.plot(x,y)
        plt.title(f"{self.name}'s path")
        plt.show()
