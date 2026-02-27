## Class structure

Classes are blueprints for custom types. They can include methods (think of them as operations), fields to store information.

Classes can be instantiated into an **instance** of that class. Inheriting all the fields and methods. 

### Class members

Anything defined inside a class is called members, those include fields and method.

A field is a variable linked to an instance of a class, a method is a function that can be executed in link to that instance.

Take the following example:

```chry
class Person {
  public string name = "John";
}

// Here we defined a class Person with a field "name"
let myPerson = new Person(); // this is an instance of Person, it has now a variable name linked to it, it can be changed like so
myPerson.name = "Mark"; // myPerson's name is now Mark

let otherPerson = new Person(); // creating a new instance of Person has nothing to do with the previous one, otherPerson's name is John, as it is the default for Person class
```

Methods work the same, only they are invariants, they are linked to the instance:

```chry
class Person {
  public string name = "John";

  public sayHello() {
    Standard.println("Hello, I'm {}.", this.name);
  }
}

let myPerson = new Person();
myPerson.name = "Mark";
let otherPerson = new Person();

myPerson.sayHello(); // prints: Hello, I'm Mark
otherPerson.sayHello(); // prints: Hello, I'm John
```
We are using the `this` keyword to refer to the instance inside it's own code.

### Constructor

A constructor is a method used when instantiating a new instance of a class, it's invoked using the `new` keyword and the name of the `class`, as seen in the previous example.


```chry
class Account {}

new Account(); // new instance created
```

By default, a class has a default constructor, that takes no arguments and does nothing.

If you wish to have your own signature in the constructor, you may define it as a method that has the same name as the class.
```chry
class Account {
  public string name;
  public Account(string name) {
    this.name = name;
    Standard.println("Account created");
  }
}

new Account("hi"); // sets name and prints: Account created
```

<div data-note>After a constructor is defined, the default constructor is no longer usable.</div>

### Access modifiers

As you probably noticed in the past code examples, we are using the `public` keyword, this is known as an access modifier, there are a few: `public`, `private` and `protected`, these define the allowed access level of a member (field or method).
- `private` members can only be accessed inside the class' own code.
- `public` can be accessed from anywhere
- `protected` are `private` but can also be accessed from class' children

### Static members

As we have seen so far, members are linked to an **instance** of a class, static members are, on the other hand, linked to the global class type object, they are centralized fields and methods, here is an example:

```chry
class Person {

  public static int counter = 0;

}

// the counter field can be accessed like this:
Person.counter += 1;
```

One typical use is to keep count of how many instance of a class have been created:

```chry
class Person {

  public static int counter = 0;
  public Person() {
    Person.counter += 1;
  }
}

new Person();
Standard.println(Person.counter); // 1
```

`Standard` itself is a class, and `println` is a public static method.
