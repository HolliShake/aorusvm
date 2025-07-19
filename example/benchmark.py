

class Human:
    def __init__(self, a, b):
        self.name = a
        self.age = b
        print("SET:", self)

class Person(Human):
    xx = 2
    
    def __init__(self, _name, _age):
        super().__init__(_name, _age)
    
    def printxx(self):
        print("xx>>", self.xx, self)
        return 1

px = Person(1, 2)

print(">>>>>>>>>>>>.", px, px.printxx())
print(Person("Doy", 12))
