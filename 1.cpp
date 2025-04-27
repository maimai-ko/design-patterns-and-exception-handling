#include <iostream>
#include <string>
#include <limits>

using namespace std;

// Exception classes
class ECommerceException : public exception
{
private:
    string message;

public:
    ECommerceException(const string &msg) : message(msg) {}
    const char *what() const noexcept override { return message.c_str(); }
};

class InvalidIDException : public ECommerceException
{
public:
    InvalidIDException() : ECommerceException("Invalid product ID") {}
};

class EmptyCartException : public ECommerceException
{
public:
    EmptyCartException() : ECommerceException("Shopping cart is empty") {}
};

class NoOrdersException : public ECommerceException
{
public:
    NoOrdersException() : ECommerceException("No orders found") {}
};

class InvalidInputException : public ECommerceException
{
public:
    InvalidInputException() : ECommerceException("Invalid input. Please enter a valid number.") {}
};

// Template function for validated input
template <typename T>
T getValidatedInput(const string &prompt)
{
    T value;
    while (true)
    {
        cout << prompt;
        cin >> value;

        if (cin.fail())
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            throw InvalidInputException();
        }

        if (cin.peek() != '\n')
        {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            throw InvalidInputException();
        }

        cin.ignore();
        return value;
    }
}

// Product class
class Product
{
private:
    int id;
    string name;
    double price;

public:
    Product(int id, const string &name, double price) : id(id), name(name), price(price) {}

    int getId() const { return id; }
    string getName() const { return name; }
    double getPrice() const { return price; }
};

// Shopping Cart Item class
class CartItem
{
private:
    Product product;
    int quantity;

public:
    CartItem(const Product &product, int quantity) : product(product), quantity(quantity) {}

    Product getProduct() const { return product; }
    int getQuantity() const { return quantity; }
    void setQuantity(int qty) { quantity = qty; }
    double getTotalPrice() const { return product.getPrice() * quantity; }
};

// Payment Strategy Interface
class PaymentStrategy
{
public:
    virtual ~PaymentStrategy() {}
    virtual void pay(double amount) = 0;
    virtual string getMethodName() const = 0;
};

// Concrete Payment Strategies
class CashPayment : public PaymentStrategy
{
public:
    void pay(double amount) override {}
    string getMethodName() const override { return "Cash"; }
};

class CardPayment : public PaymentStrategy
{
public:
    void pay(double amount) override {}
    string getMethodName() const override { return "Credit/Debit Card"; }
};

class GCashPayment : public PaymentStrategy
{
public:
    void pay(double amount) override {}
    string getMethodName() const override { return "GCash"; }
};

// Shopping Cart class (Singleton)
class ShoppingCart
{
private:
    static ShoppingCart *instance;
    CartItem **items;
    int capacity;
    int count;

    ShoppingCart() : capacity(10), count(0)
    {
        items = new CartItem *[capacity];
    }

public:
    static ShoppingCart *getInstance()
    {
        if (!instance)
        {
            instance = new ShoppingCart();
        }
        return instance;
    }

    ~ShoppingCart()
    {
        for (int i = 0; i < count; i++)
        {
            delete items[i];
        }
        delete[] items;
    }

    void addProduct(const Product &product, int quantity = 1)
    {
        for (int i = 0; i < count; i++)
        {
            if (items[i]->getProduct().getId() == product.getId())
            {
                items[i]->setQuantity(items[i]->getQuantity() + quantity);
                return;
            }
        }

        if (count == capacity)
        {
            capacity *= 2;
            CartItem **newItems = new CartItem *[capacity];
            for (int i = 0; i < count; i++)
            {
                newItems[i] = items[i];
            }
            delete[] items;
            items = newItems;
        }

        items[count++] = new CartItem(product, quantity);
    }

    void displayCart() const
    {
        if (count == 0)
        {
            cout << "Your shopping cart is empty.\n";
            return;
        }

        cout << "\nShopping Cart:\n";
        cout << "---------------------------------------------------------\n";
        cout << "ID   Name            Price   Qty  Total\n";
        cout << "---------------------------------------------------------\n";

        double total = 0;
        for (int i = 0; i < count; i++)
        {
            const Product &p = items[i]->getProduct();
            double itemTotal = items[i]->getTotalPrice();
            total += itemTotal;

            cout.width(4);
            cout << p.getId();
            cout << "  ";
            cout.width(14);
            cout << left << p.getName();
            cout.width(7);
            cout << right << (int)p.getPrice();
            cout.width(5);
            cout << right << items[i]->getQuantity();
            cout.width(7);
            cout << right << (int)itemTotal << "\n";
        }

        cout << "---------------------------------------------------------\n";
        cout << "Total: " << (int)total << "\n";
        cout << "---------------------------------------------------------\n";
    }

    double getTotalAmount() const
    {
        if (count == 0)
            throw EmptyCartException();
        double total = 0;
        for (int i = 0; i < count; i++)
        {
            total += items[i]->getTotalPrice();
        }
        return total;
    }

    void clearCart()
    {
        for (int i = 0; i < count; i++)
        {
            delete items[i];
        }
        count = 0;
    }

    bool isEmpty() const
    {
        return count == 0;
    }

    CartItem **getItems() const { return items; }
    int getItemCount() const { return count; }
};

ShoppingCart *ShoppingCart::instance = nullptr;

// Order class
class Order
{
private:
    int orderId;
    string paymentMethod;
    CartItem **items;
    int itemCount;
    double totalAmount;

public:
    Order(int id, string method, CartItem **cartItems, int count, double total)
        : orderId(id), paymentMethod(method), itemCount(count), totalAmount(total)
    {
        items = new CartItem *[itemCount];
        for (int i = 0; i < itemCount; i++)
        {
            items[i] = new CartItem(*cartItems[i]);
        }
    }

    ~Order()
    {
        for (int i = 0; i < itemCount; i++)
        {
            delete items[i];
        }
        delete[] items;
    }

    void display() const
    {
        cout << "\nOrder ID: " << orderId << "\n";
        cout << "Payment Method: " << paymentMethod << "\n";
        cout << "Products:\n";
        cout << "ID   Name            Price   Qty\n";
        for (int i = 0; i < itemCount; i++)
        {
            Product p = items[i]->getProduct();
            cout.width(4);
            cout << p.getId();
            cout << "  ";
            cout.width(14);
            cout << left << p.getName();
            cout.width(7);
            cout << right << (int)p.getPrice();
            cout.width(5);
            cout << right << items[i]->getQuantity() << "\n";
        }
        cout << "Total Amount: " << (int)totalAmount << "\n";
    }
};

// Order storage
Order *orders[100];
int orderCount = 0;

void viewOrders()
{
    if (orderCount == 0)
    {
        throw NoOrdersException();
    }
    cout << "\n===== Order History =====\n";
    for (int i = 0; i < orderCount; i++)
    {
        cout << "---------------------------------\n";
        orders[i]->display();
    }
    cout << "---------------------------------\n";
}

int main()
{
    const int productCount = 5;
    Product *products[productCount] = {
        new Product(1, "Laptop", 999.99),
        new Product(2, "Smartphone", 599.99),
        new Product(3, "Headphones", 99.99),
        new Product(4, "Mouse", 19.99),
        new Product(5, "Keyboard", 49.99)};

    int nextOrderId = 1;

    while (true)
    {
        try
        {
            cout << "\n===== Online Store Menu =====\n";
            cout << "1. View Products\n";
            cout << "2. View Shopping Cart\n";
            cout << "3. View Orders\n";
            cout << "4. Exit\n";

            int choice;
            try
            {
                choice = getValidatedInput<int>("Enter your choice (1-4): ");
                if (choice < 1 || choice > 4)
                {
                    throw ECommerceException("Invalid menu choice. Please select 1-4.");
                }
            }
            catch (const ECommerceException &e)
            {
                cerr << "Error: " << e.what() << endl;
                continue;
            }

            if (choice == 1)
            {
                char addMore;
                do
                {
                    cout << "\nAvailable Products:\n";
                    cout << "---------------------------------\n";
                    cout << "ID   Name            Price\n";
                    cout << "---------------------------------\n";

                    for (int i = 0; i < productCount; i++)
                    {
                        cout.width(4);
                        cout << products[i]->getId();
                        cout << "  ";
                        cout.width(14);
                        cout << left << products[i]->getName();
                        cout.width(7);
                        cout << right << (int)products[i]->getPrice() << "\n";
                    }
                    cout << "---------------------------------\n";

                    int id;
                    try
                    {
                        id = getValidatedInput<int>("Enter the ID of the product you want to add to the shopping cart: ");
                    }
                    catch (const ECommerceException &e)
                    {
                        cerr << "Error: " << e.what() << endl;
                        continue;
                    }

                    Product *selectedProduct = nullptr;
                    for (int i = 0; i < productCount; i++)
                    {
                        if (products[i]->getId() == id)
                        {
                            selectedProduct = products[i];
                            break;
                        }
                    }

                    if (!selectedProduct)
                    {
                        cerr << "Error: Invalid product ID\n";
                        continue;
                    }

                    ShoppingCart::getInstance()->addProduct(*selectedProduct);
                    cout << "Product added successfully!\n";

                    cout << "Do you want to add another product? (Y/N): ";
                    cin >> addMore;
                    addMore = toupper(addMore);
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                } while (addMore == 'Y');
            }
            else if (choice == 2)
            {
                ShoppingCart *cart = ShoppingCart::getInstance();
                cart->displayCart();

                if (!cart->isEmpty())
                {
                    cout << "Do you want to check out all the products? (Y/N): ";
                    char checkoutChoice;
                    cin >> checkoutChoice;
                    checkoutChoice = toupper(checkoutChoice);
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    if (checkoutChoice == 'Y')
                    {
                        cout << "\nSelect payment method:\n";
                        cout << "1. Cash\n";
                        cout << "2. Credit/Debit Card\n";
                        cout << "3. GCash\n";

                        int paymentChoice;
                        paymentChoice = getValidatedInput<int>("Enter your choice (1-3): ");

                        PaymentStrategy *strategy = nullptr;
                        switch (paymentChoice)
                        {
                        case 1:
                            strategy = new CashPayment();
                            break;
                        case 2:
                            strategy = new CardPayment();
                            break;
                        case 3:
                            strategy = new GCashPayment();
                            break;
                        }

                        double total = cart->getTotalAmount();
                        strategy->pay(total);

                        orders[orderCount++] = new Order(nextOrderId++, strategy->getMethodName(), cart->getItems(), cart->getItemCount(), total);

                        cout << "\nYou have successfully checked out the products!\n";

                        cart->clearCart();
                        delete strategy;
                    }
                }
            }
            else if (choice == 3)
            {
                viewOrders();
            }
            else if (choice == 4)
            {
                cout << "Thank you for shopping with us!\n";
                break;
            }
        }
        catch (const exception &e)
        {
            cerr << "Error: " << e.what() << "\n";
        }
    }

    for (int i = 0; i < productCount; i++)
    {
        delete products[i];
    }
    for (int i = 0; i < orderCount; i++)
    {
        delete orders[i];
    }

    return 0;
}
