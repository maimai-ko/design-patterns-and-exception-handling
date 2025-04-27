#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <iomanip>

using namespace std;

// Exception classes
class ECommerceException : public exception
{
private:
    string message;

public:
    ECommerceException(const string &msg) : message(msg) {}
    const char *what() const noexcept override
    {
        return message.c_str();
    }
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
        cout << "--------------------------------------------------------------\n";
        cout << left << setw(8) << "ID"
             << setw(20) << "Name"
             << setw(10) << "Price"
             << setw(8) << "Qty"
             << setw(10) << "Total" << "\n";
        cout << "--------------------------------------------------------------\n";

        double total = 0;
        for (int i = 0; i < count; i++)
        {
            const Product &p = items[i]->getProduct();
            double itemTotal = items[i]->getTotalPrice();
            total += itemTotal;

            cout << left << setw(8) << p.getId()
                 << setw(20) << p.getName()
                 << "$" << setw(9) << fixed << setprecision(0) << p.getPrice()
                 << setw(8) << items[i]->getQuantity()
                 << "$" << fixed << setprecision(0) << itemTotal << "\n";
        }
        cout << "--------------------------------------------------------------\n";
        cout << "Total: $" << fixed << setprecision(0) << total << "\n";
        cout << "--------------------------------------------------------------\n";
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

    string getCartContents() const
    {
        if (count == 0)
            return "";
        string contents;
        for (int i = 0; i < count; i++)
        {
            const Product &p = items[i]->getProduct();
            contents += to_string(p.getId()) + "\t" + p.getName() + "\t" +
                        to_string(p.getPrice()) + "\t" + to_string(items[i]->getQuantity()) + "\n";
        }
        return contents;
    }
};

ShoppingCart *ShoppingCart::instance = nullptr;

void viewOrders()
{
    ifstream logFile("orders.log");
    if (!logFile.is_open())
    {
        throw NoOrdersException();
    }

    string line;
    bool hasOrders = false;

    cout << "\n===== Order History =====\n";

    while (getline(logFile, line))
    {
        if (line.find("[LOG] -> Order ID: ") != string::npos)
        {
            hasOrders = true;

            // Extract order ID
            size_t idStart = line.find("Order ID: ") + 10;
            size_t idEnd = line.find(" has been");
            string orderId = line.substr(idStart, idEnd - idStart);

            // Extract payment method
            size_t methodStart = line.find("using ") + 6;
            size_t methodEnd = line.find(".");
            string paymentMethod = line.substr(methodStart, methodEnd - methodStart);

            cout << left << setw(8) << "ID"
                 << setw(20) << "Name"
                 << setw(10) << "Price"
                 << setw(8) << "Qty" << "\n";

            // Read and display products until empty line or end of file
            while (getline(logFile, line) && !line.empty())
            {
                // Parse product line (format: ID Name Price Quantity)
                size_t tab1 = line.find('\t');
                size_t tab2 = line.find('\t', tab1 + 1);
                size_t tab3 = line.find('\t', tab2 + 1);

                if (tab1 != string::npos && tab2 != string::npos && tab3 != string::npos)
                {
                    string id = line.substr(0, tab1);
                    string name = line.substr(tab1 + 1, tab2 - tab1 - 1);
                    string price = line.substr(tab2 + 1, tab3 - tab2 - 1);
                    string qty = line.substr(tab3 + 1);

                    cout << id << "\t" << name << "\t$" << price << "\t" << qty << "\n";
                }
            }

            // Read and display total if available
            if (getline(logFile, line) && line.find("Total Amount: $") != string::npos)
            {
                cout << line << "\n";
            }
        }
    }

    if (!hasOrders)
    {
        throw NoOrdersException();
    }

    cout << "---------------------------------\n";
    logFile.close();
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
                    cout << "ID\tName\t\tPrice\n";
                    cout << "---------------------------------\n";

                    for (int i = 0; i < productCount; i++)
                    {
                        cout << products[i]->getId() << "\t" << products[i]->getName()
                             << "\t$" << products[i]->getPrice() << "\n";
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

                    try
                    {
                        char input;
                        cout << "Do you want to add another product? (Y/N): ";
                        cin >> input;
                        addMore = toupper(input);
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        if (addMore != 'Y' && addMore != 'N')
                        {
                            throw ECommerceException("Please enter Y or N.");
                        }
                    }
                    catch (const ECommerceException &e)
                    {
                        cerr << "Error: " << e.what() << endl;
                        addMore = 'N';
                    }
                } while (addMore == 'Y');
            }
            else if (choice == 2)
            {
                ShoppingCart *cart = ShoppingCart::getInstance();
                cart->displayCart();

                if (!cart->isEmpty())
                {
                    char checkoutChoice;
                    try
                    {
                        cout << "Do you want to check out all the products? (Y/N): ";
                        cin >> checkoutChoice;
                        checkoutChoice = toupper(checkoutChoice);
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        if (checkoutChoice != 'Y' && checkoutChoice != 'N')
                        {
                            throw ECommerceException("Please enter Y or N.");
                        }
                    }
                    catch (const ECommerceException &e)
                    {
                        cerr << "Error: " << e.what() << endl;
                        continue;
                    }

                    if (checkoutChoice == 'Y')
                    {
                        cout << "\nSelect payment method:\n";
                        cout << "1. Cash\n";
                        cout << "2. Credit/Debit Card\n";
                        cout << "3. GCash\n";

                        int paymentChoice;
                        try
                        {
                            paymentChoice = getValidatedInput<int>("Enter your choice (1-3): ");
                            if (paymentChoice < 1 || paymentChoice > 3)
                            {
                                throw ECommerceException("Invalid payment method. Please select 1-3.");
                            }
                        }
                        catch (const ECommerceException &e)
                        {
                            cerr << "Error: " << e.what() << endl;
                            continue;
                        }

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

                        time_t now = time(nullptr);
                        string orderId = "ORD" + to_string(now);

                        // In the checkout section (around line 400 in your original code):
                        ofstream logFile("orders.log", ios::app);
                        if (logFile.is_open())
                        {
                            logFile << "[LOG] -> Order ID: " << orderId
                                    << " has been successfully checked out and paid using "
                                    << strategy->getMethodName() << ".\n";
                            logFile << cart->getCartContents();
                            logFile << "Total Amount: $" << total << "\n\n"; // Add total and extra newline
                            logFile.close();
                        }

                        cout << "\nYou have successfully checked out the products!\n";
                        cout << "Order ID: " << orderId << "\n";
                        cout << "Payment Method: " << strategy->getMethodName() << "\n";
                        cout << "Total Amount: $" << total << "\n";

                        cart->clearCart();
                        delete strategy;
                    }
                }
            }
            else if (choice == 3)
            {
                try
                {
                    viewOrders();
                }
                catch (const ECommerceException &e)
                {
                    cerr << "Error: " << e.what() << endl;
                }
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

    return 0;
}