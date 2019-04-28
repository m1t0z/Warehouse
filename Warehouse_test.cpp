#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "Warehouse.h"

template<class... Args>
std::shared_ptr<Product> MakeProduct(Args&&... args)
{
  return std::make_shared<Product>(Product{std::forward<Args>(args)...});  
}

TEST_CASE("adding products to the warehouse") {
  
  Warehouse wh;

  SUBCASE("products with new ids should be added") {
    CHECK(wh.AddProduct(MakeProduct("id1", "producer", "name", 1u)) == true);
    CHECK(wh.AddProduct(MakeProduct("id2", "producer", "name", 1u)) == true);
    CHECK(wh.AddProduct(MakeProduct("id3", "producer", "name", 1u)) == true);
  }

  SUBCASE("products with existing ids should not be added") {
    REQUIRE(wh.AddProduct(MakeProduct("id1", "producer", "name", 1u)));
    REQUIRE(wh.AddProduct(MakeProduct("id2", "producer", "name", 1u)));
    REQUIRE(wh.AddProduct(MakeProduct("id3", "producer", "name", 1u)));  

    CHECK(wh.AddProduct(MakeProduct("id1", "producer", "name", 1u)) == false);
    CHECK(wh.AddProduct(MakeProduct("id2", "producer", "name", 1u)) == false);
    CHECK(wh.AddProduct(MakeProduct("id3", "producer", "name", 1u)) == false);  
  }
}

TEST_CASE("finding products inside the warehouse by the given product id") {
  
  Warehouse wh;

  SUBCASE("empty warehouse => nothing found") {
    CHECK(!wh.FindProductById("sample_id"));
  }

  SUBCASE("warehouse contains several products, but not the one which we are looking for") {
    REQUIRE(wh.AddProduct(MakeProduct("id1", "producer", "name", 1u)));
    REQUIRE(wh.AddProduct(MakeProduct("id2", "producer", "name", 1u)));
    REQUIRE(wh.AddProduct(MakeProduct("id3", "producer", "name", 1u))); 

    CHECK(!wh.FindProductById("id42"));
  }  

  SUBCASE("warehouse contains several products, including the one which we are looking for") {
    auto product1 = MakeProduct("id1", "producer", "name", 1u);
    REQUIRE(wh.AddProduct(product1));
    REQUIRE(wh.AddProduct(MakeProduct("id2", "producer", "name", 1u)));
    REQUIRE(wh.AddProduct(MakeProduct("id3", "producer", "name", 1u))); 

    CHECK(wh.FindProductById("id1") == product1);
  }  
}

TEST_CASE("finding products inside the warehouse by the given producer") {
  
  Warehouse wh;

  SUBCASE("empty warehouse => nothing found") {
    std::vector<Warehouse::ProductPtr> foundProducts;
    CHECK(wh.FindProductsByProducer("sample_producer", std::back_inserter(foundProducts)) == 0);
    CHECK(foundProducts.size() == 0);
  }

  SUBCASE("warehouse contains several products, but none of them from the producer we are looking for") {
    REQUIRE(wh.AddProduct(MakeProduct("id1", "producer1", "name", 1u)));
    REQUIRE(wh.AddProduct(MakeProduct("id2", "producer2", "name", 1u)));
    REQUIRE(wh.AddProduct(MakeProduct("id3", "producer3", "name", 1u))); 

    std::vector<Warehouse::ProductPtr> foundProducts;
    CHECK(wh.FindProductsByProducer("producer42", std::back_inserter(foundProducts)) == 0);
    CHECK(foundProducts.size() == 0);
  } 


  SUBCASE("warehouse contains products of multiple producers we are searching for") {
    auto producer1_productA = MakeProduct("id1", "producer1", "A", 1u);
    auto producer2_productB = MakeProduct("id2", "producer2", "B", 1u);
    auto producer2_productC = MakeProduct("id3", "producer2", "C", 1u);
    auto producer3_productD = MakeProduct("id4", "producer3", "D", 1u);
    auto producer3_productE = MakeProduct("id5", "producer3", "E", 1u);
    auto producer3_productF = MakeProduct("id6", "producer3", "F", 1u);

    // add products in the producer independent order
    REQUIRE(wh.AddProduct(producer3_productD));
    REQUIRE(wh.AddProduct(producer2_productB));
    REQUIRE(wh.AddProduct(producer3_productF));
    REQUIRE(wh.AddProduct(producer1_productA));
    REQUIRE(wh.AddProduct(producer3_productE));
    REQUIRE(wh.AddProduct(producer2_productC));

    // find products of the producer1
    {
      std::set<Warehouse::ProductPtr> foundProducts;
      auto outIt = std::inserter(foundProducts, foundProducts.end());
      CHECK(wh.FindProductsByProducer("producer1", outIt) == 1);
      CHECK(foundProducts.size() == 1);
      CHECK(foundProducts.count(producer1_productA) == 1);
    }

    // find products of the producer2
    {
      std::set<Warehouse::ProductPtr> foundProducts;
      auto outIt = std::inserter(foundProducts, foundProducts.end()); 
      CHECK(wh.FindProductsByProducer("producer2", outIt) == 2);
      CHECK(foundProducts.size() == 2);
      CHECK(foundProducts.count(producer2_productB) == 1);
      CHECK(foundProducts.count(producer2_productC) == 1);
    }    

    // find products of the producer3
    {
      std::set<Warehouse::ProductPtr> foundProducts;
      auto outIt = std::inserter(foundProducts, foundProducts.end());
      CHECK(wh.FindProductsByProducer("producer3", outIt) == 3);
      CHECK(foundProducts.size() == 3);
      CHECK(foundProducts.count(producer3_productD) == 1);
      CHECK(foundProducts.count(producer3_productE) == 1);
      CHECK(foundProducts.count(producer3_productF) == 1);
    }        
  }
}

TEST_CASE("removing products from the warehouse by the given product id") {
  
  Warehouse wh;

  SUBCASE("empty warehouse => nothing to remove") {
    CHECK(wh.RemoveProductById("sample_id") == 0);
  }

  SUBCASE("trying to remove a product that is not inside the warehouse") {
    REQUIRE(wh.AddProduct(MakeProduct("id1", "producer1", "name", 1u)));
    REQUIRE(wh.AddProduct(MakeProduct("id2", "producer2", "name", 1u)));
    REQUIRE(wh.AddProduct(MakeProduct("id3", "producer3", "name", 1u))); 

    CHECK(wh.RemoveProductById("sample_id") == 0);
  }

  SUBCASE("removing products that are inside the warehouse") {
    auto producerA_product1 = MakeProduct("id1", "producerA", "1", 1u);
    auto producerA_product2 = MakeProduct("id2", "producerA", "2", 1u);
    auto producerA_product3 = MakeProduct("id3", "producerA", "3", 1u);

    REQUIRE(wh.AddProduct(producerA_product1));
    REQUIRE(wh.AddProduct(producerA_product2));
    REQUIRE(wh.AddProduct(producerA_product3));

    // remove the first product
    {
      CHECK(wh.RemoveProductById("id1") == 1);
      
      // check the state of the warehouse after the product removal ...
      // ..via FindProductById method 
      CHECK(!wh.FindProductById("id1"));
      CHECK(wh.FindProductById("id2") == producerA_product2);
      CHECK(wh.FindProductById("id3") == producerA_product3);
      // ...via FindProductsByProducer method
      std::set<Warehouse::ProductPtr> foundProducts;
      auto outIt = std::inserter(foundProducts, foundProducts.end());
      CHECK(wh.FindProductsByProducer("producerA", outIt) == 2);
      CHECK(foundProducts.size() == 2);
      CHECK(foundProducts.count(producerA_product2) == 1);
      CHECK(foundProducts.count(producerA_product3) == 1);
    }

    // remove the second product
    {
      CHECK(wh.RemoveProductById("id2") == 1);
      
      // check the state of the warehouse after the product removal ...
      // ..via FindProductById method 
      CHECK(!wh.FindProductById("id1"));
      CHECK(!wh.FindProductById("id2"));
      CHECK(wh.FindProductById("id3") == producerA_product3);
      // ...via FindProductsByProducer method
      std::set<Warehouse::ProductPtr> foundProducts;
      auto outIt = std::inserter(foundProducts, foundProducts.end());
      CHECK(wh.FindProductsByProducer("producerA", outIt) == 1);
      CHECK(foundProducts.size() == 1);
      CHECK(foundProducts.count(producerA_product3) == 1);
    }    

    // remove the third product
    {
      CHECK(wh.RemoveProductById("id3") == 1);
      
      // check the state of the warehouse after the product removal ...
      // ..via FindProductById method 
      CHECK(!wh.FindProductById("id1"));
      CHECK(!wh.FindProductById("id2"));
      CHECK(!wh.FindProductById("id3"));
      // ...via FindProductsByProducer method
      std::set<Warehouse::ProductPtr> foundProducts;
      auto outIt = std::inserter(foundProducts, foundProducts.end());
      CHECK(wh.FindProductsByProducer("producerA", outIt) == 0);
      CHECK(foundProducts.size() == 0);
    }     
  }     
}