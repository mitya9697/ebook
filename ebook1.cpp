#include <cassert>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std::literals;

class BookManager {
public:
    BookManager() {
        user_count_by_page_.resize(1001, 0);
    }

    void Read(int user_id, int page_id) {
        int old_page_id = page_by_user_id_[user_id];
        page_by_user_id_[user_id] = page_id;
        for (int i = old_page_id + 1; i <= page_id; ++i) {
            ++user_count_by_page_[i];
        }
    }

    double Cheer(int user_id) const {
        if (page_by_user_id_.count(user_id) == 0) {
            return 0.;
        } else if (page_by_user_id_.size() == 1) {
            return 1.;
        }
        int page = page_by_user_id_.at(user_id);

        return static_cast<double>(page_by_user_id_.size() - user_count_by_page_[page]) / (page_by_user_id_.size() - 1);
    }

private:
    std::unordered_map<int, int> page_by_user_id_;
    std::vector<int> user_count_by_page_;
};

class RequestHandler {
public:
    RequestHandler(std::istream& input) {
        int n = 0;
        input >> n;
        for (int i = 0; i < n; ++i) {
            RequestStat request;
            input >> request.type;
            if (request.type == "READ"s) {
                input >> request.user_id >> *request.page_id;
            } else if (request.type == "CHEER"s) {
                input >> request.user_id;
            }
            requests_.push_back(std::move(request));
        }
    }
    RequestHandler(RequestHandler&& other)
        : requests_(std::move(other.requests_))
    { }
    RequestHandler& operator=(RequestHandler&& rhs) {
        if (this != &rhs) {
            requests_ = std::move(rhs.requests_);
        }
        return *this;
    }

    void RequestImpl(BookManager& book_manager, std::ostream& output) const {
        for (const auto& request : requests_) {
            if (request.type == "READ"s) {
                book_manager.Read(request.user_id, *request.page_id);
            } else if (request.type == "CHEER"s) {
                output << std::setprecision(6) << book_manager.Cheer(request.user_id) << std::endl;
            }
        }
    }

private:
    struct RequestStat {
        std::string type;
        int user_id;
        std::optional<int> page_id;
    };

    std::vector<RequestStat> requests_;
};

std::string RequestResult(const RequestHandler& rh, BookManager& bm) {
    std::ostringstream oss;
    rh.RequestImpl(bm, oss);
    return oss.str();
}

void Tests() {
    {
        std::istringstream iss{
            "1\n"s
            + "CHEER 2\n"s
        };

        RequestHandler request_handler(iss);
        BookManager book_manager;
        assert(RequestResult(request_handler, book_manager) == "0\n"s);        
    }

    {
        std::istringstream iss{
            "2\n"s
            + "READ 2 10\n"s
            + "CHEER 2\n"s
        };

        RequestHandler request_handler(iss);
        BookManager book_manager;
        assert(RequestResult(request_handler, book_manager) != "0\n"s);
        assert(RequestResult(request_handler, book_manager) == "1\n"s);
    }

    {
        std::istringstream iss{
            "4\n"s
            + "READ 1 5\n"s
            + "READ 2 10\n"s
            + "READ 3 15\n"s
            + "CHEER 2\n"s
        };

        RequestHandler request_handler(iss);
        BookManager book_manager;
        assert(RequestResult(request_handler, book_manager) != "0\n"s);
        assert(RequestResult(request_handler, book_manager) != "1\n"s);
        assert(RequestResult(request_handler, book_manager) == "0.5\n"s);

        iss = std::istringstream{
            "1\n"s
            + "CHEER 3\n"s
        };
        request_handler = RequestHandler{iss};
        assert(RequestResult(request_handler, book_manager) == "1\n"s);

        iss = std::istringstream{
            "2\n"s
            + "READ 2 15\n"s
            + "CHEER 3\n"s
        };
        request_handler = RequestHandler{iss};
        assert(RequestResult(request_handler, book_manager) == "0.5\n"s);
    }

    {
        std::istringstream iss{            
            "12\n"s
            + "CHEER 5\n"s
            + "READ 1 10\n"s
            + "CHEER 1\n"s
            + "READ 2 5\n"s
            + "READ 3 7\n"s
            + "CHEER 2\n"s
            + "CHEER 3\n"s
            + "READ 3 10\n"s
            + "CHEER 3\n"s
            + "READ 3 11\n"s
            + "CHEER 3\n"s
            + "CHEER 1\n"s
        };

        RequestHandler request_handler(iss);
        BookManager book_manager;
        assert(
            RequestResult(request_handler, book_manager) 
                == 
            "0\n"s
            + "1\n"s
            + "0\n"s
            + "0.5\n"s
            + "0.5\n"s
            + "1\n"s
            + "0.5\n"s
        );
    }

    {
        std::istringstream iss{            
            "5\n"s
            + "READ 1 10\n"s
            + "READ 2 10\n"s
            + "READ 3 10\n"s
            + "READ 5 11\n"s
            + "CHEER 5\n"s
        };

        RequestHandler request_handler(iss);
        BookManager book_manager;
        assert(RequestResult(request_handler, book_manager) == "1\n"s);
    }
}

int main() {
    Tests();

    RequestHandler request_handler(std::cin);

    BookManager book_manager;

    request_handler.RequestImpl(book_manager, std::cout);
}