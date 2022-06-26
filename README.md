# Project 02: Message Queue

This is [Project 02] of [CSE.30341.FA21].

## Students

1. Tim Fitzpatrick (tfitzpa3@nd.edu)
2. Parker Scanlon (pscanlo2@nd.edu)


## Brainstorming

The following are questions that should help you in thinking about how to
approach implementing [Project 02].  For this project, responses to these
brainstorming questions **are not required**.

### Request

1. What data must be allocated and deallocated for each `Request` structure?

2. What does a valid **HTTP** request look like?

### Queue

1. What data must be allocated and deallocated for each `Queue` structure?

2. How will you implement **mutual exclusion**?

3. How will you implement **signaling**?
    
4. What are the **critical sections**?

### Client

1. What data must be allocated and deallocated for each `MessageQueue`
   structure?

2. What should happen when the user **publishes** a message?

3. What should happen when the user **retrieves** a message?

4. What should happen when the user **subscribes** to a topic?

5. What should happen when the user **unsubscribes** to a topic?
    
6. What needs to happen when the user **starts** the `MessageQueue`?

7. What needs to happen when the user **stops** the `MessageQueue`?

8. How many internal **threads** are required?

9. What is the purpose of each internal **thread**?

10. What `MessageQueue` attribute needs to be **protected** from **concurrent**
    access?
    
## Demonstration

> https://youtu.be/vk3ihaencVY

## Errata

> Describe any known errors, bugs, or deviations from the requirements.

There are no errors/deviations from the requirements.

[Project 02]:       https://www3.nd.edu/~pbui/teaching/cse.30341.fa21/project02.html
[CSE.30341.FA21]:   https://www3.nd.edu/~pbui/teaching/cse.30341.fa21/
