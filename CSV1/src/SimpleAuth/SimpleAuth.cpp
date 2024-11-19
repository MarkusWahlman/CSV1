#include "SimpleAuth.h"

#include <iostream>

SimpleAuth::SimpleAuth()
    : 
    m_App(nullptr),
    m_Auth(nullptr),
    m_User(nullptr),
    m_Version(0)
{
    /* https://tomeko.net/online_tools/cpp_text_escape.php?lang=en */

    /*OLD project details, server closed, authentication no longer work*/
    std::string configJson =
        "{\n"
        "  \"project_info\": {\n"
        "    \"project_number\": \"1003157219251\",\n"
        "    \"project_id\": \"mayumi-726bf\",\n"
        "    \"storage_bucket\": \"mayumi-726bf.appspot.com\"\n"
        "  },\n"
        "  \"client\": [\n"
        "    {\n"
        "      \"client_info\": {\n"
        "        \"mobilesdk_app_id\": \"1:1003157219251:android:da7d520961e91e16840dc6\",\n"
        "        \"android_client_info\": {\n"
        "          \"package_name\": \"mayumi.exe\"\n"
        "        }\n"
        "      },\n"
        "      \"oauth_client\": [\n"
        "        {\n"
        "          \"client_id\": \"1003157219251-64huc350t2klsrpt5fv252gldnqpufib.apps.googleusercontent.com\",\n"
        "          \"client_type\": 3\n"
        "        }\n"
        "      ],\n"
        "      \"api_key\": [\n"
        "        {\n"
        "          \"current_key\": \"AIzaSyCasd8OnxwvZ0aHNOlQztxd2RT9cztPJs0\"\n"
        "        }\n"
        "      ],\n"
        "      \"services\": {\n"
        "        \"appinvite_service\": {\n"
        "          \"other_platform_oauth_client\": [\n"
        "            {\n"
        "              \"client_id\": \"1003157219251-64huc350t2klsrpt5fv252gldnqpufib.apps.googleusercontent.com\",\n"
        "              \"client_type\": 3\n"
        "            }\n"
        "          ]\n"
        "        }\n"
        "      }\n"
        "    }\n"
        "  ],\n"
        "  \"configuration_version\": \"1\"\n"
        "}";

    firebase::AppOptions* AppOptions = firebase::AppOptions::LoadFromJsonConfig(configJson.c_str());
    firebase::App* fba = firebase::App::Create(*AppOptions);
    m_App = fba;

    m_Auth = firebase::auth::Auth::GetAuth(m_App);
}

float SimpleAuth::GetVersion()
{
    if (m_Version)
        return m_Version;

    firebase::firestore::Firestore* firestore = firebase::firestore::Firestore::GetInstance();
    firebase::firestore::CollectionReference versionCollection = firestore->Collection("version");
    firebase::firestore::DocumentReference versionDocument = versionCollection.Document("version");

    firebase::Future<firebase::firestore::DocumentSnapshot> versionDocumentFuture = versionDocument.Get();
    //Wait for document and check status
    while (true)
    {
        if (versionDocumentFuture.status() == firebase::kFutureStatusComplete)
        {
            if (versionDocumentFuture.error() == 0)
                break;
            else
                return m_Version;
        }

        if (versionDocumentFuture.status() == firebase::kFutureStatusInvalid)
            return m_Version;
    }
    firebase::firestore::DocumentSnapshot versionDocumentSnapshot = *versionDocumentFuture.result();

    firebase::firestore::FieldValue fValue = versionDocumentSnapshot.Get("currentVersion");
    if (fValue.is_double())
        m_Version = (float)fValue.double_value();
    else
        m_Version = (float)fValue.integer_value();

    return m_Version;
}

//bool SimpleAuth::SignIn(std::string username, std::string password)
//{
//    m_Username = username;
//    m_Password = password;
//
//    firebase::Future<firebase::auth::User*> result =
//        m_Auth->SignInWithEmailAndPassword(username.c_str(), password.c_str());
//
//    while (true)
//    {
//        if (result.status() == firebase::kFutureStatusComplete) {
//            if (result.error() == firebase::auth::kAuthErrorNone) {
//                m_User = *result.result();
//                if(IsActiveUser())
//                    return true;
//                m_User->~User();
//                m_User = nullptr;
//                return false;
//                
//            }
//        }
//        if (result.status() == firebase::kFutureStatusInvalid || result.error() != firebase::auth::AuthError::kAuthErrorNone)
//        {
//            return false;
//        }
//    }
//}

//bool SimpleAuth::IsSignedIn()
//{
//    // Grab the result of the latest sign-in attempt.
//    firebase::Future<firebase::auth::User*> future =
//        m_Auth->SignInWithEmailAndPasswordLastResult();
//
//    if (future.status() == firebase::kFutureStatusInvalid ||
//        (future.status() == firebase::kFutureStatusComplete &&
//            future.error() != firebase::auth::kAuthErrorNone))
//    {
//        return false;
//    }
//
//    // We're signed in if the most recent result was successful.
//    return future.status() == firebase::kFutureStatusComplete &&
//        future.error() == firebase::auth::kAuthErrorNone && *future.result() == m_User;
//}

//bool SimpleAuth::IsActiveUser()
//{
//    if (m_User == nullptr) return false;
//
//    firebase::firestore::Firestore* firestore = firebase::firestore::Firestore::GetInstance();
//    firebase::firestore::CollectionReference customersCollectionRef = firestore->Collection("customers");
//    firebase::firestore::DocumentReference userDocRef = customersCollectionRef.Document(m_User->uid());
//    firebase::firestore::CollectionReference subsCollectionRef = userDocRef.Collection("subscriptions");
//    firebase::Future<firebase::firestore::QuerySnapshot> subsQueryFuture = subsCollectionRef.Get();
//
//    //Wait for document and check status
//    while (true)
//    {
//        if (subsQueryFuture.status() == firebase::kFutureStatusComplete)
//        {
//            if (subsQueryFuture.error() == 0)
//                break;
//            else
//                return false;
//        }
//
//        if (subsQueryFuture.status() == firebase::kFutureStatusInvalid)
//            return false;
//    }
//
//    firebase::firestore::QuerySnapshot subsQuery = *subsQueryFuture.result();
//    
//    std::vector<firebase::firestore::DocumentSnapshot> docSnapshots = subsQuery.documents();
//    if (docSnapshots.empty()) return false;
//    firebase::firestore::FieldValue fValue = docSnapshots[0].Get("status");
//    return fValue.string_value() == "active"; 
//}