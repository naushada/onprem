import { HttpClient, HttpHeaders, HttpParams } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Observable } from 'rxjs';
import { environment } from 'src/environments/environment';

@Injectable({
  providedIn: 'root'
})
export class HttpService {

  
  private apiURL:string = "";

  constructor(private http:HttpClient) {

    if(!environment.production) {
      this.apiURL = "http://localhost:8080"; 
    }
   }

   
   httpOptions = {
    headers: new HttpHeaders({
      'Content-Type': 'application/json'
    })
  }

  /*
  Response Format will be as below JSON Object
  {
    "status": "success/failure", ----> Mandatory
    "details": "failure reason", ----> Conditional
    "response": JSON Document    ----> Optional
  }
  */
  getlogininfo(id:string, pwd: string): Observable<string> {

    let param = "";

    if(id.length > 0 && pwd.length > 0) {
      param = `userid=${id}&password=${pwd}`;
    }

    let uri: string = "";
    if(this.apiURL.length > 0) {
      uri = this.apiURL + "/api/v1/dms/account";
    } else {
      uri = "/api/v1/dms/account";
    }

    const options = {params: new HttpParams({fromString: param})};
    return this.http.get<string>(uri, options);
  }

  /**
  Request:
   {
      "userid": "",
      "password": "",
      "roles": ["advanced", "basic", "etc"]
   }
   Response:
   {
    "status": "success/failure", ----> Mandatory
    "details": "failure reason", ----> Conditional
    "response": JSON Document    ----> Optional
  } 
   */
  createaccount(input:string): Observable<string> {
    let uri: string = "";
    if(this.apiURL.length > 0) {
      uri = this.apiURL + "/api/v1/dms/account";
    } else {
      uri = "/api/v1/dms/account";
    }
    return this.http.post<string>(uri, input, this.httpOptions);
  }

  /**
  Request:
   {
      "serialnumber": "",
      "productcode": "",
      "producttype": ""
   }
   Response:
   {
    "status": "success/failure", ----> Mandatory
    "details": "failure reason", ----> Conditional
    "response": JSON Document    ----> Optional
  } 
   */
  registerdevice(input:string): Observable<string> {
    let uri: string = "";
    if(this.apiURL.length > 0) {
      uri = this.apiURL + "/api/v1/dms/register";
    } else {
      uri = "/api/v1/dms/register";
    }
    return this.http.post<string>(uri, input, this.httpOptions);
  }


  /**
  Request:
   {
      "imagename": "",
      "osversion": "",
      "osbuildnumber": "",
      "producttype": ""
   }
   Response:
   {
    "status": "success/failure", ----> Mandatory
    "details": "failure reason", ----> Conditional
    "response": JSON Document    ----> Optional
  } 
   */
  uploadsoftwarerelease(input:string): Observable<string> {
    let uri: string = "";
    if(this.apiURL.length > 0) {
      uri = this.apiURL + "/api/v1/dms/swrelease";
    } else {
      uri = "/api/v1/dms/swrelease";
    }
    return this.http.post<string>(uri, input, this.httpOptions);
  }

  /*

   Response:
   {
    "status": "success/failure", ----> Mandatory
    "details": "failure reason", ----> Conditional
    "response": Array of JSON Object    ----> Optional
  } 
  */
  getsoftwarerelease(): Observable<string> {
    let uri: string = "";
    if(this.apiURL.length > 0) {
      uri = this.apiURL + "/api/v1/dms/swrelease";
    } else {
      uri = "/api/v1/dms/swrelease";
    }
    return this.http.get<string>(uri);
  }
  
}
