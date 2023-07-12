#include "catalogTestShared.hpp"

#include <api/catalog/handlers.hpp>
#include <gtest/gtest.h>
#include <rbac/mockRbac.hpp>
#include <testsCommon.hpp>

const std::string rCommand {"dummy cmd"};
const std::string rOrigin {"Dummy org module"};

bool successAuth(const std::string&)
{
    return true;
}

bool failAuth(const std::string&)
{
    return false;
}

class Handlers : public ::testing::Test
{

protected:
    std::shared_ptr<MockRBAC> rbac;

    void SetUp() override
    {
        initLogging();
        rbac = std::make_shared<MockRBAC>();
    }

    void TearDown() override {}
};

TEST_F(Handlers, resourceGet)
{
    auto config = getConfig();
    auto catalog = std::make_shared<api::catalog::Catalog>(config);

    base::Name name({api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder),
                     successName.parts()[1],
                     successName.parts()[2]});

    api::Handler cmd;
    EXPECT_CALL(*rbac, getAuthFn(testing::_, testing::_)).Times(2).WillRepeatedly(testing::Return(successAuth));
    ASSERT_NO_THROW(cmd = api::catalog::handlers::resourceGet(catalog, rbac));
    json::Json params {fmt::format("{{\"name\": \"{}\", \"format\": \"json\", \"role\": \"any\"}}", name.fullName()).c_str()};

    ASSERT_NO_THROW(cmd(api::wpRequest::create(rCommand, rOrigin, params)));
    auto response = cmd(api::wpRequest::create(rCommand, rOrigin, params));
    const auto expectedData = json::Json(R"({"status":"OK","content":"{\"name\":\"decoder/name/ok\"}"})");

    // check response
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 0);
    ASSERT_FALSE(response.message().has_value());
    ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
                                             << "Actual: " << response.data().prettyStr() << std::endl;
}

TEST_F(Handlers, resourceGet_Persist)
{
    api::Handler cmd;
    EXPECT_CALL(*rbac, getAuthFn(testing::_, testing::_)).Times(2).WillRepeatedly(testing::Return(successAuth));
    base::Name name({api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder),
                     successName.parts()[1],
                     successName.parts()[2]});
    {
        auto config = getConfig();
        auto catalog = std::make_shared<api::catalog::Catalog>(config);

        ASSERT_NO_THROW(cmd = api::catalog::handlers::resourceGet(catalog, rbac));
    }
    json::Json params {fmt::format("{{\"name\": \"{}\", \"format\": \"json\", \"role\": \"any\"}}", name.fullName()).c_str()};
    ASSERT_NO_THROW(cmd(api::wpRequest::create(rCommand, rOrigin, params)));
    auto response = cmd(api::wpRequest::create(rCommand, rOrigin, params));
    const auto expectedData = json::Json(R"({"status":"OK","content":"{\"name\":\"decoder/name/ok\"}"})");

    // check response
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 0);
    ASSERT_FALSE(response.message().has_value());
    ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
                                             << "Actual: " << response.data().prettyStr() << std::endl;
}

// TEST_F(Handlers, resourceGet_MissingName)
// {
//     auto config = getConfig();
//     auto catalog = std::make_shared<api::catalog::Catalog>(config);

//     ASSERT_NO_THROW(api::catalog::handlers::resourceGet(catalog, rbac));
//     json::Json params {R"({"format": "json"})"};
//     auto response = api::catalog::handlers::resourceGet(catalog, rbac)(api::wpRequest::create(rCommand, rOrigin,
//     params)); const auto expectedData = json::Json(R"({"status":"ERROR","error":"Missing /name parameter"})");

//     // check response
//     ASSERT_TRUE(response.isValid());
//     ASSERT_EQ(response.error(), 0);
//     ASSERT_FALSE(response.message().has_value());
//     ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
//                                              << "Actual: " << response.data().prettyStr() << std::endl;
// }

// TEST_F(Handlers, resourceGet_MissingFormat)
// {
//     auto config = getConfig();
//     auto catalog = std::make_shared<api::catalog::Catalog>(config);
//     base::Name name({api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder),
//                      successName.parts()[1],
//                      successName.parts()[2]});

//     ASSERT_NO_THROW(api::catalog::handlers::resourceGet(catalog, rbac));
//     json::Json params {fmt::format(R"({{"name": "{}"}})", name.fullName()).c_str()};
//     auto response = api::catalog::handlers::resourceGet(catalog, rbac)(api::wpRequest::create(rCommand, rOrigin,
//     params)); const auto expectedData = json::Json(R"({"status":"ERROR","error":"Missing or invalid /format
//     parameter"})");

//     // check response
//     ASSERT_TRUE(response.isValid());
//     ASSERT_EQ(response.error(), 0);
//     ASSERT_FALSE(response.message().has_value());
//     ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
//                                              << "Actual: " << response.data().prettyStr() << std::endl;
// }

// TEST_F(Handlers, resourceGet_CatalogError)
// {
//     auto config = getConfig();
//     auto catalog = std::make_shared<api::catalog::Catalog>(config);
//     base::Name name({api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder),
//                      failName.parts()[1],
//                      failName.parts()[2]});
//     ASSERT_NO_THROW(api::catalog::handlers::resourceGet(catalog, rbac));
//     json::Json params {fmt::format("{{\"name\": \"{}\", \"format\": \"json\"}}", name.fullName()).c_str()};
//     auto response = api::catalog::handlers::resourceGet(catalog, rbac)(api::wpRequest::create(rCommand, rOrigin,
//     params)); const auto expectedData = json::Json(
//         R"({"status":"ERROR","error":"Content 'decoder/name/fail' could not be obtained from store: error"})");

//     // check response
//     ASSERT_TRUE(response.isValid());
//     ASSERT_EQ(response.error(), 0);
//     ASSERT_FALSE(response.message().has_value());
//     ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
//                                              << "Actual: " << response.data().prettyStr() << std::endl;
// }

// TEST_F(Handlers, resourceGet_InvalidFormat)
// {
//     auto config = getConfig();
//     auto catalog = std::make_shared<api::catalog::Catalog>(config);
//     base::Name name({api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder),
//                      successName.parts()[1],
//                      successName.parts()[2]});
//     ASSERT_NO_THROW(api::catalog::handlers::resourceGet(catalog, rbac));
//     json::Json params {fmt::format("{{\"name\": \"{}\", \"format\": \"invalid\"}}", name.fullName()).c_str()};
//     auto response = api::catalog::handlers::resourceGet(catalog, rbac)(api::wpRequest::create(rCommand, rOrigin,
//     params)); const auto expectedData = json::Json(R"({"status":"ERROR","error":"Missing or invalid /format
//     parameter"})");

//     // check response
//     ASSERT_TRUE(response.isValid());
//     ASSERT_EQ(response.error(), 0);
//     ASSERT_FALSE(response.message().has_value());
//     ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
//                                              << "Actual: " << response.data().prettyStr() << std::endl;
// }

// TEST_F(Handlers, resourceGet_InvalidName)
// {
//     auto config = getConfig();
//     auto catalog = std::make_shared<api::catalog::Catalog>(config);
//     ASSERT_NO_THROW(api::catalog::handlers::resourceGet(catalog, rbac));
//     json::Json params {"{\"name\": \"invalid\", \"format\": \"json\"}"};
//     auto response = api::catalog::handlers::resourceGet(catalog, rbac)(api::wpRequest::create(rCommand, rOrigin,
//     params)); const auto expectedData = json::Json(R"({"status":"ERROR","error":"Invalid collection type
//     \"invalid\""})");

//     // check response
//     ASSERT_TRUE(response.isValid());
//     ASSERT_EQ(response.error(), 0);
//     ASSERT_FALSE(response.message().has_value());
//     ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
//                                              << "Actual: " << response.data().prettyStr() << std::endl;
// }

// TEST_F(Handlers, resourcePost)
// {
//     auto config = getConfig();
//     auto catalog = std::make_shared<api::catalog::Catalog>(config);

//     base::Name name(api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder));

//     api::Handler cmd;
//     ASSERT_NO_THROW(cmd = api::catalog::handlers::resourcePost(catalog, rbac));
//     json::Json params;
//     params.setObject();
//     params.setString(name.fullName(), "/type");
//     params.setString("json", "/format");
//     params.setString(successJson.str(), "/content");
//     ASSERT_NO_THROW(cmd(api::wpRequest::create(rCommand, rOrigin, params)));
//     auto response = cmd(api::wpRequest::create(rCommand, rOrigin, params));
//     const auto expectedData = json::Json(R"({"status":"OK"})");

//     // check response
//     ASSERT_TRUE(response.isValid());
//     ASSERT_EQ(response.error(), 0);
//     ASSERT_FALSE(response.message().has_value());
//     ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
//                                              << "Actual: " << response.data().prettyStr() << std::endl;
// }

// TEST_F(Handlers, resourcePost_Persist)
// {
//     api::Handler cmd;
//     base::Name name(api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder));
//     {
//         auto config = getConfig();
//         auto catalog = std::make_shared<api::catalog::Catalog>(config);

//         ASSERT_NO_THROW(cmd = api::catalog::handlers::resourcePost(catalog, rbac));
//     }
//     json::Json params;
//     params.setObject();
//     params.setString(name.fullName(), "/type");
//     params.setString("json", "/format");
//     params.setString(successJson.str(), "/content");
//     ASSERT_NO_THROW(cmd(api::wpRequest::create(rCommand, rOrigin, params)));
//     auto response = cmd(api::wpRequest::create(rCommand, rOrigin, params));
//     const auto expectedData = json::Json(R"({"status":"OK"})");

//     // check response
//     ASSERT_TRUE(response.isValid());
//     ASSERT_EQ(response.error(), 0);
//     ASSERT_FALSE(response.message().has_value());
//     ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
//                                              << "Actual: " << response.data().prettyStr() << std::endl;
// }

// TEST_F(Handlers, resourcePost_NotCollectionType)
// {
//     auto config = getConfig();
//     auto catalog = std::make_shared<api::catalog::Catalog>(config);

//     api::Handler cmd;
//     ASSERT_NO_THROW(cmd = api::catalog::handlers::resourcePost(catalog, rbac));
//     json::Json params;
//     params.setObject();
//     params.setString(successName.fullName(), "/type");
//     params.setString("json", "/format");
//     params.setString(successJson.str(), "/content");
//     ASSERT_NO_THROW(cmd(api::wpRequest::create(rCommand, rOrigin, params)));
//     auto response = cmd(api::wpRequest::create(rCommand, rOrigin, params));
//     const auto expectedData = json::Json(R"({"status":"ERROR","error":"Missing /type parameter or is invalid"})");

//     // check response
//     ASSERT_TRUE(response.isValid());
//     ASSERT_EQ(response.error(), 0);
//     ASSERT_FALSE(response.message().has_value());
//     ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
//                                              << "Actual: " << response.data().prettyStr() << std::endl;
// }

// TEST_F(Handlers, resourcePost_MissingType)
// {
//     auto config = getConfig();
//     auto catalog = std::make_shared<api::catalog::Catalog>(config);

//     api::Handler cmd;
//     ASSERT_NO_THROW(cmd = api::catalog::handlers::resourcePost(catalog, rbac));
//     json::Json params;
//     params.setObject();
//     params.setString("json", "/format");
//     params.setString(successJson.str(), "/content");
//     ASSERT_NO_THROW(cmd(api::wpRequest::create(rCommand, rOrigin, params)));
//     auto response = cmd(api::wpRequest::create(rCommand, rOrigin, params));
//     const auto expectedData = json::Json(R"({"status":"ERROR","error":"Missing /type parameter or is invalid"})");

//     // check response
//     ASSERT_TRUE(response.isValid());
//     ASSERT_EQ(response.error(), 0);
//     ASSERT_FALSE(response.message().has_value());
//     ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
//                                              << "Actual: " << response.data().prettyStr() << std::endl;
// }

// TEST_F(Handlers, resourcePost_MissingFormat)
// {
//     auto config = getConfig();
//     auto catalog = std::make_shared<api::catalog::Catalog>(config);

//     base::Name name(api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder));

//     api::Handler cmd;
//     ASSERT_NO_THROW(cmd = api::catalog::handlers::resourcePost(catalog, rbac));
//     json::Json params;
//     params.setObject();
//     params.setString(name.fullName(), "/type");
//     params.setString(successJson.str(), "/content");
//     ASSERT_NO_THROW(cmd(api::wpRequest::create(rCommand, rOrigin, params)));
//     auto response = cmd(api::wpRequest::create(rCommand, rOrigin, params));
//     const auto expectedData = json::Json(R"({"status":"ERROR","error":"Missing /format parameter or is invalid"})");

//     // check response
//     ASSERT_TRUE(response.isValid());
//     ASSERT_EQ(response.error(), 0);
//     ASSERT_FALSE(response.message().has_value());
//     ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
//                                              << "Actual: " << response.data().prettyStr() << std::endl;
// }

// TEST_F(Handlers, resourcePost_MissingContent)
// {
//     auto config = getConfig();
//     auto catalog = std::make_shared<api::catalog::Catalog>(config);

//     base::Name name(api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder));

//     api::Handler cmd;
//     ASSERT_NO_THROW(cmd = api::catalog::handlers::resourcePost(catalog, rbac));
//     json::Json params;
//     params.setObject();
//     params.setString(name.fullName(), "/type");
//     params.setString("json", "/format");
//     ASSERT_NO_THROW(cmd(api::wpRequest::create(rCommand, rOrigin, params)));
//     auto response = cmd(api::wpRequest::create(rCommand, rOrigin, params));
//     const auto expectedData = json::Json(R"({"status":"ERROR","error":"Missing /content parameter"})");

//     // check response
//     ASSERT_TRUE(response.isValid());
//     ASSERT_EQ(response.error(), 0);
//     ASSERT_FALSE(response.message().has_value());
//     ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
//                                              << "Actual: " << response.data().prettyStr() << std::endl;
// }

// TEST_F(Handlers, resourcePut)
// {
//     auto config = getConfig();
//     auto catalog = std::make_shared<api::catalog::Catalog>(config);

//     base::Name name({api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder),
//                      successName.parts()[1],
//                      successName.parts()[2]});

//     api::Handler cmd;
//     ASSERT_NO_THROW(cmd = api::catalog::handlers::resourcePut(catalog, rbac));
//     json::Json params;
//     params.setObject();
//     params.setString(name.fullName(), "/name");
//     params.setString("json", "/format");
//     params.setString(successJson.str(), "/content");
//     ASSERT_NO_THROW(cmd(api::wpRequest::create(rCommand, rOrigin, params)));
//     auto response = cmd(api::wpRequest::create(rCommand, rOrigin, params));
//     const auto expectedData = json::Json(R"({"status":"OK"})");

//     // check response
//     ASSERT_TRUE(response.isValid());
//     ASSERT_EQ(response.error(), 0);
//     ASSERT_FALSE(response.message().has_value());
//     ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
//                                              << "Actual: " << response.data().prettyStr() << std::endl;
// }

// TEST_F(Handlers, resourcePut_Persist)
// {
//     api::Handler cmd;
//     base::Name name({api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder),
//                      successName.parts()[1],
//                      successName.parts()[2]});
//     {
//         auto config = getConfig();
//         auto catalog = std::make_shared<api::catalog::Catalog>(config);

//         ASSERT_NO_THROW(cmd = api::catalog::handlers::resourcePut(catalog, rbac));
//     }
//     json::Json params;
//     params.setObject();
//     params.setString(name.fullName(), "/name");
//     params.setString("json", "/format");
//     params.setString(successJson.str(), "/content");
//     ASSERT_NO_THROW(cmd(api::wpRequest::create(rCommand, rOrigin, params)));
//     auto response = cmd(api::wpRequest::create(rCommand, rOrigin, params));
//     const auto expectedData = json::Json(R"({"status":"OK"})");

//     // check response
//     ASSERT_TRUE(response.isValid());
//     ASSERT_EQ(response.error(), 0);
//     ASSERT_FALSE(response.message().has_value());
//     ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
//                                              << "Actual: " << response.data().prettyStr() << std::endl;
// }

// TEST_F(Handlers, resourcePut_Collection)
// {
//     auto config = getConfig();
//     auto catalog = std::make_shared<api::catalog::Catalog>(config);

//     base::Name name(api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder));

//     api::Handler cmd;
//     ASSERT_NO_THROW(cmd = api::catalog::handlers::resourcePut(catalog, rbac));
//     json::Json params;
//     params.setObject();
//     params.setString(name.fullName(), "/name");
//     params.setString("json", "/format");
//     params.setString(successJson.str(), "/content");
//     ASSERT_NO_THROW(cmd(api::wpRequest::create(rCommand, rOrigin, params)));
//     auto response = cmd(api::wpRequest::create(rCommand, rOrigin, params));
//     const auto expectedData =
//         json::Json(R"({"status":"ERROR","error":"Invalid resource type 'collection' for PUT operation"})");

//     // check response
//     ASSERT_TRUE(response.isValid());
//     ASSERT_EQ(response.error(), 0);
//     ASSERT_FALSE(response.message().has_value());
//     ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
//                                              << "Actual: " << response.data().prettyStr() << std::endl;
// }

// TEST_F(Handlers, resourcePut_MissingName)
// {
//     auto config = getConfig();
//     auto catalog = std::make_shared<api::catalog::Catalog>(config);

//     api::Handler cmd;
//     ASSERT_NO_THROW(cmd = api::catalog::handlers::resourcePut(catalog, rbac));
//     json::Json params;
//     params.setObject();
//     params.setString("json", "/format");
//     params.setString(successJson.str(), "/content");
//     ASSERT_NO_THROW(cmd(api::wpRequest::create(rCommand, rOrigin, params)));
//     auto response = cmd(api::wpRequest::create(rCommand, rOrigin, params));
//     const auto expectedData = json::Json(R"({"status":"ERROR","error":"Missing /name parameter"})");

//     // check response
//     ASSERT_TRUE(response.isValid());
//     ASSERT_EQ(response.error(), 0);
//     ASSERT_FALSE(response.message().has_value());
//     ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
//                                              << "Actual: " << response.data().prettyStr() << std::endl;
// }

// TEST_F(Handlers, resourcePut_MissingFormat)
// {
//     auto config = getConfig();
//     auto catalog = std::make_shared<api::catalog::Catalog>(config);

//     base::Name name(api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder));

//     api::Handler cmd;
//     ASSERT_NO_THROW(cmd = api::catalog::handlers::resourcePut(catalog, rbac));
//     json::Json params;
//     params.setObject();
//     params.setString(name.fullName(), "/name");
//     params.setString(successJson.str(), "/content");
//     ASSERT_NO_THROW(cmd(api::wpRequest::create(rCommand, rOrigin, params)));
//     auto response = cmd(api::wpRequest::create(rCommand, rOrigin, params));
//     const auto expectedData = json::Json(R"({"status":"ERROR","error":"Missing or invalid /format parameter"})");

//     // check response
//     ASSERT_TRUE(response.isValid());
//     ASSERT_EQ(response.error(), 0);
//     ASSERT_FALSE(response.message().has_value());
//     ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
//                                              << "Actual: " << response.data().prettyStr() << std::endl;
// }

// TEST_F(Handlers, resourcePut_MissingContent)
// {
//     auto config = getConfig();
//     auto catalog = std::make_shared<api::catalog::Catalog>(config);

//     base::Name name(api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder));

//     api::Handler cmd;
//     ASSERT_NO_THROW(cmd = api::catalog::handlers::resourcePut(catalog, rbac));
//     json::Json params;
//     params.setObject();
//     params.setString(name.fullName(), "/name");
//     params.setString("json", "/format");
//     ASSERT_NO_THROW(cmd(api::wpRequest::create(rCommand, rOrigin, params)));
//     auto response = cmd(api::wpRequest::create(rCommand, rOrigin, params));
//     const auto expectedData = json::Json(R"({"status":"ERROR","error":"Missing /content parameter"})");

//     // check response
//     ASSERT_TRUE(response.isValid());
//     ASSERT_EQ(response.error(), 0);
//     ASSERT_FALSE(response.message().has_value());
//     ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
//                                              << "Actual: " << response.data().prettyStr() << std::endl;
// }

// TEST_F(Handlers, resourceDelete)
// {
//     auto config = getConfig();
//     auto catalog = std::make_shared<api::catalog::Catalog>(config);

//     base::Name name({api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder),
//                      successName.parts()[1],
//                      successName.parts()[2]});

//     api::Handler cmd;
//     ASSERT_NO_THROW(cmd = api::catalog::handlers::resourceDelete(catalog, rbac));
//     json::Json params;
//     params.setObject();
//     params.setString(name.fullName(), "/name");
//     ASSERT_NO_THROW(cmd(api::wpRequest::create(rCommand, rOrigin, params)));
//     auto response = cmd(api::wpRequest::create(rCommand, rOrigin, params));
//     const auto expectedData = json::Json(R"({"status":"OK"})");

//     // check response
//     ASSERT_TRUE(response.isValid());
//     ASSERT_EQ(response.error(), 0);
//     ASSERT_FALSE(response.message().has_value());
//     ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
//                                              << "Actual: " << response.data().prettyStr() << std::endl;
// }

// TEST_F(Handlers, resourceDelete_Persist)
// {
//     api::Handler cmd;
//     base::Name name({api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder),
//                      successName.parts()[1],
//                      successName.parts()[2]});
//     {
//         auto config = getConfig();
//         auto catalog = std::make_shared<api::catalog::Catalog>(config);

//         ASSERT_NO_THROW(cmd = api::catalog::handlers::resourceDelete(catalog, rbac));
//     }
//     json::Json params;
//     params.setObject();
//     params.setString(name.fullName(), "/name");
//     ASSERT_NO_THROW(cmd(api::wpRequest::create(rCommand, rOrigin, params)));
//     auto response = cmd(api::wpRequest::create(rCommand, rOrigin, params));
//     const auto expectedData = json::Json(R"({"status":"OK"})");

//     // check response
//     ASSERT_TRUE(response.isValid());
//     ASSERT_EQ(response.error(), 0);
//     ASSERT_FALSE(response.message().has_value());
//     ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
//                                              << "Actual: " << response.data().prettyStr() << std::endl;
// }

// TEST_F(Handlers, resourceDelete_Collection)
// {
//     auto config = getConfig();
//     auto catalog = std::make_shared<api::catalog::Catalog>(config);

//     base::Name name(api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder));

//     api::Handler cmd;
//     ASSERT_NO_THROW(cmd = api::catalog::handlers::resourceDelete(catalog, rbac));
//     json::Json params;
//     params.setObject();
//     params.setString(name.fullName(), "/name");
//     ASSERT_NO_THROW(cmd(api::wpRequest::create(rCommand, rOrigin, params)));
//     auto response = cmd(api::wpRequest::create(rCommand, rOrigin, params));
//     const auto expectedData = json::Json(R"({"status":"OK"})");

//     // check response
//     ASSERT_TRUE(response.isValid());
//     ASSERT_EQ(response.error(), 0);
//     ASSERT_FALSE(response.message().has_value());
//     ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
//                                              << "Actual: " << response.data().prettyStr() << std::endl;
// }

// TEST_F(Handlers, resourceDelete_MissingName)
// {
//     auto config = getConfig();
//     auto catalog = std::make_shared<api::catalog::Catalog>(config);

//     api::Handler cmd;
//     ASSERT_NO_THROW(cmd = api::catalog::handlers::resourceDelete(catalog, rbac));
//     json::Json params;
//     params.setObject();
//     ASSERT_NO_THROW(cmd(api::wpRequest::create(rCommand, rOrigin, params)));
//     auto response = cmd(api::wpRequest::create(rCommand, rOrigin, params));
//     const auto expectedData = json::Json(R"({"status":"ERROR","error":"Missing /name parameter"})");

//     // check response
//     ASSERT_TRUE(response.isValid());
//     ASSERT_EQ(response.error(), 0);
//     ASSERT_FALSE(response.message().has_value());
//     ASSERT_EQ(response.data(), expectedData) << "Expected: " << expectedData.prettyStr() << std::endl
//                                              << "Actual: " << response.data().prettyStr() << std::endl;
// }

// TEST_F(Handlers, registerHandlers)
// {
//     auto config = getConfig();
//     auto catalog = std::make_shared<api::catalog::Catalog>(config);
//     auto api = std::make_shared<api::Api>();
//     ASSERT_NO_THROW(api::catalog::handlers::registerHandlers(catalog, api));
// }
