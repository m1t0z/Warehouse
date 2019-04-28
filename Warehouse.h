#include <cassert>
#include <string>
#include <mutex>
#include <unordered_map>
#include <map>
#include <memory>
		
//! Description of a single product item
struct Product 
{
  using Id = std::string;
  using Producer = std::string;
  using Name = std::string;
  using Price = unsigned;

  Id id;             //!< unique id of the product among all products
  Producer producer; //!< producer of the product
  Name name;         //!< name of the product
  Price price{0};    //!< price of the product
};

//! Warehouse that contains products
//! 
//! All public methods are thread-safe
class Warehouse
{
public:
  using ProductPtr = std::shared_ptr<const Product>;

  //! Add the given product \p pProduct to the warehouse
  //! 
  //! If the warehouse already contains a product with the same id
  //! as id of the \p pProduct then \p pProduct will not be added.
  //!
  //! Algorithm's time complexity:
  //! - average case: O(logN) 
  //! - worst case:   O(N)
  //! 
  //! \pre \p pProduct is not an empty smart-pointer
  //!
  //! \return true if product was added, false otherwise
  bool AddProduct(const ProductPtr &pProduct)
  {
    // pre-conditions
    assert(pProduct);

    std::lock_guard<std::mutex> lock(mMutex);

    // Time complexity of the std::unordered_map::emplace(): 
    // - average: O(1)
    // - worst:   O(N)
    auto [it, success] = mProductsWithMetasById.emplace(pProduct->id, ProductWithMeta(pProduct));
    if(success)
    {
      // Time complexity of the std::multimap::emplace(): O(logN)
      it->second.meta.it_productsByProducer = 
        mProductsByProducer.emplace(pProduct->producer, pProduct);
    }
    return success;

    // Time complexity of the method:
    // - average: O(1) + O(logN) = O(logN)
    // - worst:   O(N) + O(logN) = O(N)
  }
  
  //! Find a product inside the warehouse by the given product id \p id
  //! 
  //! Algorithm's time complexity:
  //! - average case: O(1) 
  //! - worst case:   O(N)
  //! 
  //! \return if the product with the given id \p id is presented inside the warehouse then 
  //!   the smart pointer to this product will be returned, 
  //!   otherwise an empty smart pointer will be returned 
  ProductPtr FindProductById(const Product::Id &id) const
  {
    std::lock_guard<std::mutex> lock(mMutex);

    // Time complexity of the std::unordered_map::find(key):
    // - average: O(1)
    // - worst:   O(N)
    auto it = mProductsWithMetasById.find(id);
    return (it != mProductsWithMetasById.end())? it->second.pProduct : ProductPtr();

    // Time complexity of the method:
    // - average: O(1)
    // - worst:   O(N) 
  }

  //! Find inside the warehouse all products of the given producer \p producer.
  //! 
  //! \param[in] producer producer of the products
  //! \param[out] out out-iterator to which the found products (if any) would be copied
  //!
  //! Algorithm's time complexity:
  //! - average case: O(max(logN,M)), where M is number of products of the given producer
  //! - worst case:   O(N)
  //! 
  //! \return number of found products
  template <typename OutputIt>
  size_t FindProductsByProducer(const Product::Producer &producer, OutputIt out) const
  { 
    std::lock_guard<std::mutex> lock(mMutex);

    // Time complexity of the std::multimap::equal_range(key): O(logN)
    auto range = mProductsByProducer.equal_range(producer);
    
    // Time complexity of copying: O(M), where M - number of products of the given producer
    // Relation between M and N: 0 <= M <= N
    size_t cntFound = 0;
    while(range.first != range.second)
    {
      *out++ = (range.first++)->second;
      cntFound++; 
    }
    return cntFound; 

    // Time complexity of the method:
    // - average: O(logN) + O(M) = O(max(logN, M))
    // - worst:   O(logN) + O(N) = O(N)
  }

  //! Remove the product with given id \p id from the warehouse
  //! 
  //! In case the product with the given id is not presented in the warehouse the state
  //! of the warehouse will not be changed
  //!
  //! Algorithm's time complexity:
  //! - average case: O(1) 
  //! - worst case:   O(N)
  //!
  //! \post @returned is 0 or 1
  //! 
  //! \return the number of removed products, which is either 1 or 0
  size_t RemoveProductById(const Product::Id &id)
  {  
    std::lock_guard<std::mutex> lock(mMutex);

    // Time complexity of the std::unordered_map::find(key):
    // - average: O(1)
    // - worst:   O(N)
    auto it = mProductsWithMetasById.find(id);
    if (it == mProductsWithMetasById.end()) 
    {
      return 0;
    }

    // Time complexity of the std::multimap::erase(iterator):
    // - average: O(1)
    // - worst:   O(N)
    mProductsByProducer.erase(it->second.meta.it_productsByProducer);

    // Time complexity of the std::unordered_map::erase(iterator):
    // - average: O(1)
    // - worst:   O(N)
    mProductsWithMetasById.erase(it);

    return 1;

    // Time complexity of the method:
    // - average: O(1) + O(1) + O(1) = O(1)
    // - worst:   O(N) + O(N) + O(N) = O(N)
  }

private:
  mutable std::mutex mMutex;
  
  std::multimap<Product::Producer, ProductPtr> mProductsByProducer;

  struct ProductWithMeta 
  {
    ProductWithMeta(ProductPtr _pProduct): pProduct(_pProduct){}

    struct Meta 
    {
      decltype(mProductsByProducer)::const_iterator it_productsByProducer;
    };

    ProductPtr pProduct;
    Meta meta;
  };    

  std::unordered_map<Product::Id, ProductWithMeta> mProductsWithMetasById;
};

