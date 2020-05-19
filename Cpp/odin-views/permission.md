# Checking permissions

Permissions are central to how many APIs work. Odin only has the ability to attach permissions to user identities, this means that anonymous requests cannot use these permissions checks.

In order to handle anonymous requests the "unsecure" path for the `odin.secure` path must implement the APIs needed for anonymous access.


## The `odin.permission` view

```json
{
    "view": "odin.permission",
    "configuration": {
        "permission": "permission1",
        "allowed": "granted-view",
        "forbidden": {
            "view": "denied-view"
        }
    }
}
```

## The `odin.permission.method` view

This view is used when different HTTP verbs require different permissions.

```json
{
    "view": "odin.permission.method",
    "configuration": {
        "OPTIONS": "example.view",
        "GET": {
            "permission": "permission1-name",
            "allowed": "example-view"
        },
        "PUT": {
            "permission": "permission2-name",
            "allowed": "example-view",
            "forbidden": "read-only-view"
        },
        "otherwise": {
            "view": "fost.response.405",
            "configuration": {
                "verbs": ["GET", "HEAD", "OPTIONS", "PUT"]
            }
        }
    }
}
```
