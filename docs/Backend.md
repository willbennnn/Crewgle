# Backend Documentation
This document will contain pretty much everything I learned while coding the backend for The Crewgle.

## Table of Contents
* [Basic Terminology](#Basic-Terminology)
* [Backend Development Tools](#Backend-Development-Tools)
  * [Drogon](#The-Drogon-Framework)
  * [SQLite](#SQLite3)
  * [OpenSSL](#OpenSSL)
  * [cpp STD](#C++-Standard-Library)
  * [CMake & Library Linking](#CMake-&-Library-Linking)
  * [Git & Github](#Git-&-GitHub)
  * [HomeBrew](#HomeBrew-(for-MacOs))
* [Backend Structure](#Backend-Structure)
  * [Database](#Database)
  * [Email](#Email)
  * [Http](#Http)
  * [Routes](#Routes)
  * [Schema](#Schema)
  * [Security](#Security)
  * [Services](#Services)
* [Miscellaneous Info](#Miscellaneous-Information)


## Basic Terminology
The Crewgle API (application programming interface) follows some basic web development principles that guide the entire backend. These prinicples include:

* **HTTP -** The *HyperText Transfer Protocol*, it operates with the classic client-server model. A user makes a *request* through the *client* (a web browser or an app eventually) for some resource, and then the *server* (the backend API) recieves the request & processes it, returning the requested *response* and a *status code*. HTTP is characterized by its use of methods (discussed further under *CRUD*), its statelessness (meaning it does not retain data about the connection between the server and the client), and its use of headers (metadata passed alongside requests and responses that include type of content, client info, etc.).
  
* **RESTful API -** RESTful APIs are an architechtural style for client-server web applications. Though some of the priniciples of the REST model are for more advances systems that Crewgle has not reached yet, the idea is to follow the REST model for all of development. The 6 guiding principles of RESTful APIs are:
  * **Uniformity -** Keep interactions standard across all components. Use standard methods, self descriptive messages, and distinct resource paths
  * **Statelessness -** Every request from the client most contain all of the information necessary to process it. THe server retains no data between requests and can make no callbacks before processing the request for additional information
  * **Client-Server Architecture -** The front-end user interface and the back-end data storage are independent of each other; both sides scale and evolve seperately
  * **Cacheability -** Servers must lable response data as cacheable or non-cacheable to reduce client-server overhead and increase response speeds
  * **Layered System -** The client cannot tell whether it is connected to directly to the end server (the backend) or an intermediate layer (such as a load balancer or security proxy)
  * **Code On Demand -** This one is slightly different as it is the only optional principle of the six, however crewgle heavily relies on it through the front end frameworks it used. It means for user functionality to be extended by executing code snippets client-side that are sent by the server as opposed to those code snippets having to be pre-implemented. The React framework relies *heavily* on this
  Of the above principles, two stand out (Layered system & cacheability) as more advanced topics. These technically are not part of the design of crewgle yet (as of July 2026), but might be eventually if the application expands beyond the scope of TAMU crew. Both principles refer to speedups and security additions needed for large scale web apps (which crewgle is not yet). Tools like Redis allow for caching of common responses, which is much quicker than calling the database for every request, while load balancers are designed to distribute traffic across multiple concurrent servers as opposed to clogging up one large server.
* **CRUD -** CRUD is an acronym for database management that is used by the REST model. Each letter also has a corresponding HTTP method & core SQL command that it maps to. The acronym stands for:
  * **Create:** Adds a new record or new information to the database (Corresponds to the `POST` HTTP method & the `INSERT` SQL command)
  * **Read:** Retrieves or searches for existing data (Corresponds to the `GET` HTTP method & the `SELECT` SQL command)
  * **Update:** Modifies or edits existing data without creating a new record (Corresponds to the [`PUT`/`PATCH`](#PUT-vs-PATCH) HTTP methods & the `UPDATE` SQL command)
  * **Delete:** Removes, cancels, or archives unwanted data (Corresponds to the `DELETE` HTTP method & the `DELETE` SQL command)


## Backend Development Stack
When developing the backend for Crewgle, a suite of tools was used that made the development process a lot quicker and easier

### The Drogon Framework
Drogon is a fast and lightweight C++ backend framework that is incredibly fast, asynchronous, and portable. It has support for utilities like Redis and an ORM that works with most of the popular open source relational databases. No specifics really need to be mentioned here since the syntax of Drogon is the core of the backends structure anyways.

### SQLite3
SQLite was chosen early on in the development process as the database of choice because it requires virtually no setup, the entire database runs locally, still has real relational database features, and has good performance for small applications.

### OpenSSL

### {whatever email api I eventually use}

### {the regatta central api eventually}

### C++ Standard Library

### CMake & Library Linking

### Git & GitHub

### HomeBrew (for MacOS)


## Backend Structure
The backend was designed in such a way that different components, methods, and helper functions can be easily explained based on the file they're located in. The file structure looks like this (updated to crewgle v0.2.3 ~ files unimportant to learning the architecture are ommitted):
```
backend/
├── build/
│   ├── CMakeCache.txt       <-- (Omitted in docs ~~ Key-Vaue store generated by cmake that stores project settings)
│   ├── crewgle.sqlite       <-- (The actual sqlite local database file ~~ to be removed when migrated to postgres or mysql)
│   ├── crewgle_backend      <-- (The binary executable to start the backend server)
│   └── Makefile             <-- (Omitted in docs ~~ The build script used to compile the code; generated by cmake based on cmakelists.txt)
├── include/
│   ├── crewgle/             <-- (The additional nested folder is added so "crewgle native" includes are listed as "crewgle/..." to avoid confusion)        
│       ├── Database.h
│       ├── Email.h
│       ├── Http.h
│       ├── Routes.h
│       ├── Schema.h
│       ├── Security.h
│       └── Services.h
├── src/
│   ├── Database.cpp
│   ├── Email.cpp
│   ├── Http.cpp
│   ├── Routes.cpp
│   ├── Schema.cpp
│   ├── Security.cpp
│   └── Services.cpp
└── CMakeLists.txt           <-- (Configures the makefile so the server is compiled correctly)     
```

### Database

### Email

### Http

### Routes

### Schema

### Security

### Services


## Miscellaneous Information
#### PUT vs PATCH
`PUT` & `PATCH` are two HTTP methods that are both used to update data, but the way they work is slightly different. `PUT` is more common in industry & performs a full replacement of a record, needing all resource fields, overwriting any missing fields to null/the default, and creating the resource if it is missing (the last point is a large part of the reason for it being more standard (along with [idempotency]; `PUT` leads to less bugs in practice). `PATCH` performs a partial modification where usually only some of the fields are changed and any missing fields in the request are ignored (not changed) on the record. `PATCH` allows for more flexibility in the request having blank fields, but can also fail if the record that the update is requested for does not exist (unlike `PUT`).  

#### Object Relational Mapper (ORM)
The legacy Crewgle backend made use of an *Object Relational Mapper (ORM)*, a software layer that maps rows in a relational database to objects in an object-oriented programming language. Instead of manually writing SQL queries, executing them, and converting the returned rows into C++ objects, the ORM performs much of this translation automatically. Each row in a database table becomes an instance of a C++ class, while each column becomes one of that object's data members (or properties). This allows the rest of the application to work with familiar C++ objects rather than raw SQL query results. Without an ORM, data is typically retrieved by executing SQL directly and manually extracting values from each returned row:
```C++
auto result = db.execSqlSync("SELECT email FROM users");
std::vector<std::string> emails;
for (const auto& row : result) {
    emails.push_back(row["email"].as<std::string>());
}
```
With an ORM, the same operation becomes more object-oriented:
```C++
std::vector<User> users = User::findAll();
std::vector<std::string> emails;
for (const auto& user : users) {
    emails.push_back(user.getEmail());
}
```
ORMs usually produce cleaner and more maintable code because the SQL logic is generated automatically, but can be more difficult to get the hang of especially if you're proficient in SQL.  

#### Blocking Operations, Threads (Concurrency), & Asynchrony (Asynchronous Functions)
Some operations in computing take longer than others. Performing a simple algbraic operation doesn't take very long (a single core of most modern CPUs could perform around 100 billion of these operations per second), but some operations like searching a mechanical hard drive for a specific piece of data can take an extremely long time. Computers execute the instructions you give them sequentially, so if one step in the instructions takes a long time (like the mechanical HDD example), the computer is "blocked" from doing anything else for a while, and is hence called a *blocking operation*. Many blocking operations stacked together could bring your computer or server to a standstill, so two solutions were devised: **Threads** and **Asynchronous Functions**  
  
Threads are small subsets of instructions that function similarly to how the human brain functions. Your brain isn't just "the brain" and each thing your body needs to do has to happen in the order that you think to do them in; You have different parts of the brain dedicated to fine motor skills, breathing, thinking, seeing, etc., all of which are operating concurrently. Threads function in essentially the same way, instead of your CPU being just "the CPU", you split into the *main thread* (which is sorta like your conciousness, it is the main line of execution and is unimpedable), and other threads which deal with time-consuming tasks like database reads and complex algorithms concurrently to the main thread instead of clogging up the CPUs list of instructions. This allows for the computer or server to keep functioning as normal even when there is a heavy load of time-consuming requests.

Asynchronous functions are actually quite similar to threads, but without requiring a change in the actual hardware or system design you are working with. Asynchronous functions are functions that are created to handle tasks that are known to be time consuming (like accessing a database), and then sort of operate in the backround while your program continues to run. Async functions return **a promise** that says "don't forget about me, i'll call you back when im ready", so that the CPU remembers that we are still waiting for a result from that function, and then once the promise is fufilled, that CPU returns back to that part of the instructions and recieves the response it was waiting for before jumping back to where it way in the instructions originally. From an outside perspective it looks very similar to concurrency, but it operates in a fundamentally different way where every part of the process that the programmer has control of still stays on the main thread (although some languages or frameworks may still use multithreading in the background).  

Threads are very powerful but also very computationally expensive, that is why asynchronous functions came along. Asynchronous functions are great for waiting for responses from slow operations while using minimal CPU power, while threads are good for concurrent execution.

#### Idempotency


#### Cross-Origin Resource Sharing (CORS) Headers


#### JavaScript Object Notation (JSON)


**Tokens & Sessions -***



