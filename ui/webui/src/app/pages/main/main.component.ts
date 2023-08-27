import { Component } from '@angular/core';
import { filter } from 'rxjs';
import { EventService } from 'src/common/event.service';

@Component({
  selector: 'app-main',
  templateUrl: './main.component.html',
  styleUrls: ['./main.component.scss']
})
export class MainComponent {

  selectedItem:string = "Config";
  role:string = "basic";
  userid: string = "";
  constructor(private event: EventService) {
    event.subscribe("user.login", (id:String, document:string) => {
      if(id == "user.login") {
        let response = JSON.parse(document);
        //console.log(response);
        //console.log(response["response.role"]);
        if(response["status"] == "success") {
          this.role = JSON.parse(response["response"])["role"];
          this.userid = JSON.parse(response["response"])["userid"];
        } else {
          this.role = "basic";
        }
      }
    });
  }

  onClick(item:string) {
    this.selectedItem = item;
  }
}
