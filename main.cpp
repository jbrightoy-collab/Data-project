#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <cctype>
using namespace std;

struct book {
    int id;
    string title;
    string author;
    int yearPub;
    string category;
    bool status;
};

template <typename key>
class libSystem {
private:
    vector<book> books;  // Database (our vectors) 

    // Lookup system (our hash table); For main: libSystem<int, books>
    unordered_map<key, int> indexMap;

    // "title" to list of positions in books
    unordered_map<string, vector<int>> titleMap;

    // "category" to list of positions in books
    unordered_map<string, vector<int>> categoryMap;

public:
    //Load function for preloaded file
    void loadFromFile(string filename)
    {
        ifstream file(filename);

        if (!file) //Error finding file
        {
            cout << "Error opening file\n";
            return;
        }

        string line; //e.g. ID,title,author,year,category,status; This is the structure of line

        while (getline(file, line)) //puts file into line
        {
            stringstream ss(line); //scans lines and breaks it piece by piece
            string temp;
            book b;

            // ID
            getline(ss, temp, ',');
            b.id = stoi(temp); //stoi is string to int

            // Title
            getline(ss, b.title, ',');

            // Author
            getline(ss, b.author, ',');

            // Year
            getline(ss, temp, ',');
            b.yearPub = stoi(temp);

            // Category
            getline(ss, b.category, ',');

            // Status
            getline(ss, temp, ',');
            b.status = stoi(temp);

            // Add to system 
            addBook(b);
        }

        file.close();
    }

    //Normalization
    string cases(string input)
    {
        for (char& c : input)
            c = tolower(c);
        return input;
    };

    //Add book
    void addBook(book newBook)
    {
        newBook.title = cases(newBook.title);
        newBook.category = cases(newBook.category);

        books.push_back(newBook); // adds book to main vector. 
        int index = books.size() - 1; // finds index of newly added book

        indexMap[newBook.id] = index; //maps id to index
        titleMap[newBook.title].push_back(index); // maps title to index
        categoryMap[newBook.category].push_back(index); //maps cate to index
    }


    //Remove book
    void removeBook(key id)
    {
        // 1. Check if the book exists
        auto findIt = indexMap.find(id);
        if (findIt == indexMap.end())
        {
            cout << "Cannot remove: No book found with ID " << id << "\n";
            return;
        }

        int indexToRemove = findIt->second;
        book bookToRemove = books[indexToRemove];

        // 2. Remove the deleted book's data from the maps
        indexMap.erase(id);

        // Remove from title map
        auto& titleVec = titleMap[bookToRemove.title];
        for (auto it = titleVec.begin(); it != titleVec.end(); ++it) {
            if (*it == indexToRemove) {
                titleVec.erase(it);
                break; // Safely exit after erasing
            }
        }
        // Cleanup ghost keys
        if (titleVec.empty()) titleMap.erase(bookToRemove.title);

        // Remove from category map
        auto& catVec = categoryMap[bookToRemove.category];
        for (auto it = catVec.begin(); it != catVec.end(); ++it) {
            if (*it == indexToRemove) {
                catVec.erase(it);
                break; // Safely exit after erasing
            }
        }
        // Cleanup ghost keys
        if (catVec.empty()) categoryMap.erase(bookToRemove.category);

        // 3. Swap and pop (if the book to remove isn't already the last one)
        int lastIndex = books.size() - 1;
        if (indexToRemove != lastIndex)
        {
            // Overwrite the removed book with the last book in the vector
            books[indexToRemove] = books[lastIndex];
            book movedBook = books[indexToRemove];

            // Update the indexMap for the moved book
            indexMap[movedBook.id] = indexToRemove;

            // Update the titleMap for the moved book
            auto& movedTitleVec = titleMap[movedBook.title];
            for (int& idx : movedTitleVec) {
                if (idx == lastIndex) {
                    idx = indexToRemove;
                    break;
                }
            }

            // Update the categoryMap for the moved book
            auto& movedCatVec = categoryMap[movedBook.category];
            for (int& idx : movedCatVec) {
                if (idx == lastIndex) {
                    idx = indexToRemove;
                    break; // Stop iterating once updated
                }
            }
        }

        // 4. Remove the duplicate last element from the vector
        books.pop_back();
        cout << "'" << bookToRemove.title << "' (ID: " << bookToRemove.id << ") was successfully removed.\n";

    }

    //Search book
    int searchById(key id)
    {
        //prevents waste by searching once
        auto find = indexMap.find(id);

        if (find == indexMap.end())
        {
            cout << "No book found with that id\n";
            return -1;
        }
        return find->second;
    }

    vector<int> searchByTitle(string title)
    {
        title = cases(title);
        auto find = titleMap.find(title);

        if (find == titleMap.end())
        {
            cout << "No book found with that title\n";
            return {};
        }
        return find->second;
    }

    vector<int> searchByCategory(string cat)
    {
        auto find = categoryMap.find(cat);

        if (find == categoryMap.end())
        {
            cout << "No book found with that category\n";
            return {};
        }
        return find->second;
    }

    //Display book
    void displayBook(int index)
    {
        if (index < 0 || index >= books.size())
        {
            cout << "Invalid book index\n";
            return;
        }

        const book& b = books[index];

        cout << "ID: " << b.id << "\n";
        cout << "Title: " << b.title << "\n";
        cout << "Year: " << b.yearPub << "\n";
        cout << "Author: " << b.author << "\n";
        cout << "Category: " << b.category << "\n";
        cout << "Status: " << (b.status ? "Available" : "Borrowed") << "\n";
    }

    void displayAllBooks()
    {
        if (books.empty())
        {
            cout << "No books in the library.\n";
            return;
        }

        for (int i = 0; i < books.size(); i++)
        {
            displayBook(i);
            cout << "-------------------\n"; // Separator between books
        }
    }
    //Mark book (Availible/Borrowed)
    //Note: make 1 as availible and 0 as borrowed
    int borrowBook(key id)
    {
		auto find = indexMap.find(id);
        if (find == indexMap.end())
        {
            cout << "No book found with that id" << endl;
			return -1; // Setting -1 as error
        }

		int index = find->second;
        book& b = books[index];

        if (b.status == 0)
        {
            cout << "Sorry, '" << b.title << "' is currently borrowed." << endl;
            return -1;
        }
            books[index].status = 0; // Mark as borrowed
			cout << "You have successfully borrowed '" << b.title << "'. Enjoy reading!" << endl;
            return 0; //Success
    }

    int returnBook(key id)
    {
        auto find = indexMap.find(id);
        if (find == indexMap.end())
        {
            cout << "No book found with that id" << endl;
            return -1; // Setting -1 as error
        }

        int index = find->second;
        book& b = books[index];

        if (b.status == 1)
        {
            cout << "Sorry, '" << b.title << "' is already available." << endl;
            return -1;
        }
        books[index].status = 1; // Mark as returned
        cout << "You have successfully returned '" << b.title << "'. Feel free to pick another book!" << endl;
        return 0; //Success
    }

};


int main()
{
    int selection;
    libSystem<int> Library;
    Library.loadFromFile("books.txt");    //Uses load function from class

    bool run = true;// This will be used for the while loop 
    while (run)
    {
        cout << "\nPlease select one of the following\n1.Add\n2.Remove\n3.Search\n4.Display\n5.Borrow\n6.Return\n7.Exit\nSelection: ";
        cin >> selection;

        if (cin.fail()) {// This is in case the put a letter instead of an integer 
            cin.clear(); // Clears the error state
            cin.ignore(10000, '\n'); // Discards the bad input
            selection = 0; // Forces the switch to hit the 'default' case safely
        }

        switch (selection) {
        case 1: { // Added curly braces for scope
            cout << "\n--- Adding a New Book ---\n";
            book newBook;

            cout << "Enter Book ID (integer): ";
            cin >> newBook.id;
            cin.ignore(10000, '\n'); // Clears the 'Enter' key from the buffer

            cout << "Enter Title: ";
            getline(cin, newBook.title);

            cout << "Enter Author: ";
            getline(cin, newBook.author);

            cout << "Enter Publication Year: ";
            cin >> newBook.yearPub;
            cin.ignore(10000, '\n'); // Clears the 'Enter' key from the buffer

            cout << "Enter Category: ";
            getline(cin, newBook.category);

            newBook.status = 1; // 1 = Available, 0 = Borrowed

            Library.addBook(newBook);
            cout << "'" << newBook.title << "' added successfully!\n";
        } break;

        case 2:
        {
            cout << "Removing...\n";
            cout << "Enter Book ID to remove: ";
            int removeID;
            cin >> removeID;

            if (cin.fail()) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "Invalid input. Please enter a valid integer ID.\n";
            }
            else {
                Library.removeBook(removeID);
            }
        }
        break;

        case 3: { // Scope for main menu case 3
            bool searching = true; // Controls our new inner loop

            while (searching) {
                cout << "\n--- Search Menu ---\n";
                cout << "1. ID\n2. Title\n3. Category\n4. Back\nType: ";

                int search;
                cin >> search;

                if (cin.fail()) {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    search = 0;
                }

                switch (search) {
                case 1: { // Scope for search case 1
                    cout << "Please enter the ID: ";
                    int ID;
                    cin >> ID;
                    int result1 = Library.searchById(ID);
                    if (result1 != -1) {
                        Library.displayBook(result1);
                    }
                } break;

                case 2: {
                    cout << "Enter the Title: ";
                    string title;
                    cin.ignore(10000, '\n');
                    getline(cin, title);
                    title = Library.cases(title);

                    vector<int> results = Library.searchByTitle(title);
                    if (!results.empty()) {
                        cout << "\n--- Search Results ---\n";
                        for (int i = 0; i < results.size(); i++) {
                            Library.displayBook(results[i]);
                            cout << "-------------------\n";
                        }
                    }
                } break;

                case 3: {
                    cout << "Enter the Category: ";
                    string cat;
                    cin.ignore(10000, '\n');
                    getline(cin, cat);
                    cat = Library.cases(cat);

                    vector<int> results = Library.searchByCategory(cat);
                    if (!results.empty()) {
                        cout << "\n--- Search Results ---\n";
                        for (int i = 0; i < results.size(); i++) {
                            Library.displayBook(results[i]);
                            cout << "-------------------\n";
                        }
                    }
                } break;

                case 4:
                    cout << "Returning to main menu...\n";
                    searching = false;
                    break;

                default:
                    cout << "Invalid choice. Please enter a number (1-4).\n";
                }
            }
        } break; // Ends the main menu's case 3 

        case 4:
            cout << "\nDisplaying...\n";
            Library.displayAllBooks();
            break;

       case 5: {
            cout << "\n--- Borrow a Book ---\n";
            cout << "Enter the ID of the book you want to borrow: ";
            int borrowId;
            cin >> borrowId;
            
            if (cin.fail()) {// making sure there is no miss inputs
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "Invalid input. Please enter a valid integer ID.\n";
            } else {
                Library.borrowBook(borrowId);
            }
        } break;

        case 6: {
            cout << "\n--- Return a Book ---\n";
            cout << "Enter the ID of the book you want to return: ";
            int returnId;
            cin >> returnId;
            
            if (cin.fail()) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "Invalid input. Please enter a valid integer ID.\n";
            } else {
                Library.returnBook(returnId);
            }
        } break;
        case 7:
            cout << "Exiting...\n";
            run = false;
            break;

        default:
            cout << "Please enter a number (1-7).\n";
        }
    }
    return 0;
}
