#pragma once
#include "firebase/auth.h"
#include "firebase/firestore.h"

#pragma warning(disable : 4996)
/*If the development was ongoing deprecated code would be updated*/

class SimpleAuth
{
public:

    SimpleAuth();

    float GetVersion();

    __forceinline bool SignIn(std::string username, std::string password)
    {
        m_Username = username;
        m_Password = password;

        firebase::Future<firebase::auth::User*> result =
            m_Auth->SignInWithEmailAndPassword_DEPRECATED(username.c_str(), password.c_str());

        while (true)
        {
            if (result.status() == firebase::kFutureStatusComplete) {
                if (result.error() == firebase::auth::kAuthErrorNone) {
                    m_User = *result.result();
                    if (IsActiveUser())
                        return true;
                    m_User->~User();
                    m_User = nullptr;
                    return false;

                }
            }
            if (result.status() == firebase::kFutureStatusInvalid || result.error() != firebase::auth::AuthError::kAuthErrorNone)
            {
                return false;
            }
        }
    }

    __forceinline bool IsSignedIn()
    {
        // Grab the result of the latest sign-in attempt.
        firebase::Future<firebase::auth::User*> future =
            m_Auth->SignInWithEmailAndPasswordLastResult_DEPRECATED();

        if (future.status() == firebase::kFutureStatusInvalid ||
            (future.status() == firebase::kFutureStatusComplete &&
                future.error() != firebase::auth::kAuthErrorNone))
        {
            return false;
        }

        // We're signed in if the most recent result was successful.
        return future.status() == firebase::kFutureStatusComplete &&
            future.error() == firebase::auth::kAuthErrorNone && *future.result() == m_User;
    }

    __forceinline bool IsActiveUser()
    {
        if (m_User == nullptr) return false;

        firebase::firestore::Firestore* firestore = firebase::firestore::Firestore::GetInstance();
        firebase::firestore::CollectionReference customersCollectionRef = firestore->Collection("customers");
        firebase::firestore::DocumentReference userDocRef = customersCollectionRef.Document(m_User->uid());
        firebase::firestore::CollectionReference subsCollectionRef = userDocRef.Collection("subscriptions");
        firebase::Future<firebase::firestore::QuerySnapshot> subsQueryFuture = subsCollectionRef.Get();

        //Wait for document and check status
        while (true)
        {
            if (subsQueryFuture.status() == firebase::kFutureStatusComplete)
            {
                if (subsQueryFuture.error() == 0)
                    break;
                else
                    return false;
            }

            if (subsQueryFuture.status() == firebase::kFutureStatusInvalid)
                return false;
        }

        firebase::firestore::QuerySnapshot subsQuery = *subsQueryFuture.result();

        std::vector<firebase::firestore::DocumentSnapshot> docSnapshots = subsQuery.documents();
        if (docSnapshots.empty()) return false;
        firebase::firestore::FieldValue fValue = docSnapshots[0].Get("status");
        return fValue.string_value() == "active";
    }
    
private:
    firebase::App* m_App;
    firebase::auth::Auth* m_Auth;
    firebase::auth::User* m_User;

    std::string m_Username;
    std::string m_Password;

    float m_Version;
};

