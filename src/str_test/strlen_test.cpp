#include <iostream>
#include <string>
#include <unordered_map>
// 基础咖啡类（被装饰的类）
class Coffee {
public:
    virtual std::string getDescription() const {
        return "Basic Coffee";
    }

    virtual double cost() const {
        return 1.0;
    }
};

// 调料装饰器类
class CoffeeDecorator : public Coffee {
protected:
    Coffee* coffee;

public:
    CoffeeDecorator(Coffee* coffee) : coffee(coffee) {}

    std::string getDescription() const override {
        return coffee->getDescription();
    }

    double cost() const override {
        return coffee->cost();
    }
};

// 具体的调料装饰器类
class MilkDecorator : public CoffeeDecorator {
public:
    MilkDecorator(Coffee* coffee) : CoffeeDecorator(coffee) {}

    std::string getDescription() const override {
        return coffee->getDescription() + ", Milk";
    }

    double cost() const override {
        return coffee->cost() + 0.5;
    }
};

class SugarDecorator : public CoffeeDecorator {
public:
    SugarDecorator(Coffee* coffee) : CoffeeDecorator(coffee) {}

    std::string getDescription() const override {
        return coffee->getDescription() + ", Sugar";
    }

    double cost() const override {
        return coffee->cost() + 0.2;
    }
};

struct AA{
    int aa;
};
int main() {
    // Coffee* basicCoffee = new Coffee();
    // Coffee* milkCoffee = new MilkDecorator(basicCoffee);
    // Coffee* milkSugarCoffee = new SugarDecorator(milkCoffee);

    // std::cout << "Description: " << milkSugarCoffee->getDescription() << std::endl;
    // std::cout << "Cost: $" << milkSugarCoffee->cost() << std::endl;

    // delete basicCoffee;
    // delete milkCoffee;
    // delete milkSugarCoffee;

    std::unordered_map<int, AA*> m;
    AA *a1 = new AA;
    a1->aa = 1;
    m.insert({a1->aa, a1});
    m.insert({a1->aa, a1});

    std::cout<<m.size()<<std::endl;
    return 0;
}